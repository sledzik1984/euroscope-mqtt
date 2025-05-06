// plugin.cpp
#include "plugin.h"
#include "Version.h"
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <filesystem>
#include <windows.h>
#include <mqtt/async_client.h>
#include <iostream>  // For debug output

namespace euroscope_mqtt {

namespace {

std::string Trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    auto end   = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos || end == std::string::npos)
        ? std::string{}
        : str.substr(start, end - start + 1);
}

std::map<std::string, std::string> ReadConfig(const std::string& path) {
    std::map<std::string, std::string> config;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        auto eq = line.find('=');
        if (eq != std::string::npos) {
            auto key   = Trim(line.substr(0, eq));
            auto value = Trim(line.substr(eq + 1));
            config[key] = value;
        }
    }
    return config;
}

std::map<std::string, std::string> LoadAirlineTelephonyMap(const std::string& filePath) {
    std::map<std::string, std::string> airlineMap;
    std::ifstream file(filePath);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string icao, name, telephony, country;
        if (std::getline(iss, icao, '\t') &&
            std::getline(iss, name, '\t') &&
            std::getline(iss, telephony, '\t') &&
            std::getline(iss, country)) {
            airlineMap[icao] = telephony;
        }
    }
    return airlineMap;
}

std::string GetPluginDirectory() {
    char path[MAX_PATH] = {0};
    HMODULE hModule = nullptr;
    if (GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            reinterpret_cast<LPCSTR>(&GetPluginDirectory),
            &hModule)) {
        GetModuleFileName(hModule, path, MAX_PATH);
        return std::filesystem::path(path).parent_path().string();
    }
    return {};
}

std::map<std::string, std::string> g_config;
std::map<std::string, std::string> g_airlineMap;

} // anonymous namespace

Plugin::Plugin()
    : EuroScopePlugIn::CPlugIn(
        EuroScopePlugIn::COMPATIBILITY_CODE,
        PLUGIN_NAME,
        PLUGIN_VERSION,
        PLUGIN_AUTHOR,
        PLUGIN_LICENSE),
      mqtt_client_(nullptr) {
    DisplayMessage("Version " + std::string(PLUGIN_VERSION) + " loaded", "Initialization");
    RegisterTagItemFunction("Send via MQTT", 1); 

    auto pluginDir  = GetPluginDirectory();
    auto configPath = pluginDir + "\\euroscope-mqtt.txt";

    if (!std::filesystem::exists(configPath)) {
        DisplayMessage("Configuration file not found: " + configPath, "Config");
        return;
    }

    g_config = ReadConfig(configPath);
    const auto& host = g_config["host"];
    const auto& port = g_config["port"];
    const auto& cid  = g_config["cid"];
    const auto& user = g_config["username"];
    const auto& pass = g_config["password"];
    const auto& airlinePath = g_config["icao_airlines_path"];

    if (!airlinePath.empty() && std::filesystem::exists(airlinePath)) {
        g_airlineMap = LoadAirlineTelephonyMap(airlinePath);
    } else {
        DisplayMessage("ICAO_Airlines.txt path missing or invalid", "Config");
    }

    if (host.empty() || port.empty() || cid.empty()) {
        DisplayMessage("Error: missing host/port/cid in configuration", "MQTT");
        return;
    }

    auto address = std::string("tcp://") + host + ":" + port;
    auto topic   = std::string("/euroscope/") + cid + "/hello";
    std::string payload = R"({"message":"hello"})";

    try {
        mqtt_client_ = std::make_unique<mqtt::async_client>(address, cid);
        mqtt::connect_options connOpts;
        if (!user.empty()) connOpts.set_user_name(user);
        if (!pass.empty()) connOpts.set_password(pass);

        mqtt_client_->connect(connOpts)->wait();
        mqtt_client_->publish(topic, payload.data(), payload.size(), 1, false)->wait();
        mqtt_client_->disconnect()->wait();
        mqtt_client_->stop_consuming();
        mqtt_client_.reset();

        DisplayMessage("Sent hello to " + topic, "MQTT");
    } catch (const mqtt::exception& exc) {
        DisplayMessage(std::string("MQTT error: ") + exc.what(), "MQTT");
    }
}

Plugin::~Plugin() {
    DisplayMessage("Plugin destructor called", "Debug");
    if (mqtt_client_) {
        try {
            mqtt_client_->stop_consuming();
            mqtt_client_->disconnect()->wait();
        } catch (const std::exception& e) {
            DisplayMessage(std::string("Exception in destructor: ") + e.what(), "Debug");
        } catch (...) {
            DisplayMessage("Unknown exception in destructor.", "Debug");
        }
        mqtt_client_.reset();
    }
    DisplayMessage("Plugin resources cleaned", "Debug");
}

void Plugin::DisplayMessage(const std::string& message, const std::string& sender) {
    DisplayUserMessage(
        PLUGIN_NAME,
        sender.c_str(),
        message.c_str(),
        true, false, false, false, false);
}

void Plugin::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
    (void)Pt; (void)Area;
    if (FunctionId != 1) return;

    auto fp = FlightPlanSelectASEL();
    if (!fp.IsValid()) {
        DisplayMessage("MQTT: No aircraft selected", "MQTT");
        return;
    }

    std::string callsign = fp.GetCallsign();
    std::string pilotname = fp.GetPilotName();
    std::string targetcontroller = fp.GetHandoffTargetControllerCallsign();
    std::string sectorentry_mins = std::to_string(static_cast<int>(fp.GetSectorEntryMinutes()));
    std::string sectorexit_mins = std::to_string(static_cast<int>(fp.GetSectorExitMinutes()));
    std::string ramflag = fp.GetRAMFlag() ? "true" : "false";
    std::string clamFlag = fp.GetCLAMFlag() ? "true" : "false";
    std::string clearance = fp.GetClearenceFlag() ? "true" : "false";
    std::string groundstate = fp.GetGroundState();



    std::string prefix = callsign.substr(0, 3);
    std::string suffix = callsign.substr(3);
    std::string telephony = g_airlineMap.contains(prefix) ? g_airlineMap[prefix] + " " + suffix : "";

    DisplayMessage("Selected callsign: " + callsign, "Debug");

    const auto& host = g_config["host"];
    const auto& port = g_config["port"];
    const auto& cid  = g_config["cid"];
    const auto& user = g_config["username"];
    const auto& pass = g_config["password"];

    if (host.empty() || port.empty() || cid.empty()) {
        DisplayMessage("MQTT: missing host/port/cid", "MQTT");
        return;
    }

    auto address = std::string("tcp://") + host + ":" + port;
    auto topic   = std::string("/euroscope/") + cid + "/selected";
    std::ostringstream oss;
    oss << R"({"callsign":")" << callsign
        << R"(", "pilotname":")" << pilotname
        << R"(", "targetcontroller":")" << targetcontroller
        << R"(", "sectorentry_mins":")" << sectorentry_mins
        << R"(", "sectorexit_mins":")" << sectorexit_mins
        << R"(", "ramflag":")" << ramflag
        << R"(", "clamFlag":")" << clamFlag
        << R"(", "clearance":")" << clearance
        << R"(", "groundstate":")" << groundstate
        << R"(", "telephony":")" << telephony << R"("})";

    auto data = oss.str();

    try {
        mqtt::async_client client(address, cid);
        mqtt::connect_options connOpts;
        if (!user.empty()) connOpts.set_user_name(user);
        if (!pass.empty()) connOpts.set_password(pass);

        client.connect(connOpts)->wait();
        client.publish(topic, data.data(), data.size(), 1, false)->wait();
        client.disconnect()->wait();
        client.stop_consuming();

        DisplayMessage("Sent MQTT for " + callsign, "MQTT");
    } catch (const mqtt::exception& exc) {
        DisplayMessage(std::string("MQTT error (tag): ") + exc.what(), "MQTT");
    }
}

} // namespace euroscope_mqtt

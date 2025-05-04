#include "plugin.h"
#include "Version.h"
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <filesystem>
#include <windows.h>
#include <mqtt/async_client.h>

namespace euroscope_mqtt {

    std::string Trim(const std::string& str) {
        auto start = str.find_first_not_of(" \t\r\n");
        auto end = str.find_last_not_of(" \t\r\n");
        return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }

    std::map<std::string, std::string> ReadConfig(const std::string& path) {
        std::map<std::string, std::string> config;
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string key = Trim(line.substr(0, eq));
                std::string value = Trim(line.substr(eq + 1));
                config[key] = value;
            }
        }
        return config;
    }

    std::string GetPluginDirectory() {
        char path[MAX_PATH];
        HMODULE hModule = nullptr;
        if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                              reinterpret_cast<LPCSTR>(&GetPluginDirectory),
                              &hModule)) {
            GetModuleFileName(hModule, path, MAX_PATH);
            std::filesystem::path fullPath(path);
            return fullPath.parent_path().string();
        }
        return "";
    }

    euroscope_mqtt::euroscope_mqtt()
        : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_AUTHOR, PLUGIN_LICENSE) {

        DisplayMessage("Version " + std::string(PLUGIN_VERSION) + " loaded", "Initialization");

        std::string pluginDir = GetPluginDirectory();
        std::string configPath = pluginDir + "\\euroscope-mqtt.txt";

        if (!std::filesystem::exists(configPath)) {
            DisplayMessage("Nie znaleziono pliku konfiguracyjnego: " + configPath, "Config");
            return;
        }

        auto config = ReadConfig(configPath);

        const auto& host = config["host"];
        const auto& port = config["port"];
        const auto& user = config["user"];
        const auto& pass = config["pass"];
        const auto& cid  = config["cid"];

        if (host.empty() || port.empty() || cid.empty()) {
            DisplayMessage("Błąd: brak host/port/cid w konfiguracji", "MQTT");
            return;
        }

        std::string address = "tcp://" + host + ":" + port;
        std::string topic = "/euroscope/" + cid + "/hello";
        std::string payload = R"({"message": "hello"})";

        try {
            mqtt::async_client client(address, cid);

            mqtt::connect_options connOpts;
            if (!user.empty()) connOpts.set_user_name(user);
            if (!pass.empty()) connOpts.set_password(pass);

            client.connect(connOpts)->wait();
            client.publish(topic, payload.c_str(), payload.size(), 1, false)->wait();
            client.disconnect()->wait();

            DisplayMessage("Wysłano hello na " + topic, "MQTT");
        } catch (const mqtt::exception& exc) {
            DisplayMessage("MQTT błąd: " + std::string(exc.what()), "MQTT");
        }
    }

    euroscope_mqtt::~euroscope_mqtt() {}

    void euroscope_mqtt::DisplayMessage(const std::string& message, const std::string& sender) {
        DisplayUserMessage(PLUGIN_NAME, sender.c_str(), message.c_str(), true, false, false, false, false);
    }

}

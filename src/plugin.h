#pragma once

#include "EuroScopePlugIn.h"
#include <string>
#include <memory>
#include <mqtt/async_client.h>

namespace euroscope_mqtt {

class Plugin : public EuroScopePlugIn::CPlugIn {
public:
    Plugin();
    ~Plugin() override;

    void DisplayMessage(const std::string& message, const std::string& sender = "euroscope-mqtt");
    void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) override;

private:
    std::unique_ptr<mqtt::async_client> mqtt_client_;
};

} // namespace euroscope_mqtt

extern "C" __declspec(dllexport) EuroScopePlugIn::CPlugIn* EuroScopePlugInInit();
extern "C" __declspec(dllexport) void EuroScopePlugInExit(EuroScopePlugIn::CPlugIn* pPlugInInstance);
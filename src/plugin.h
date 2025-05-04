// plugin.h
#pragma once

#include "EuroScopePlugIn.h"
#include <string>

namespace euroscope_mqtt {
    class euroscope_mqtt : public EuroScopePlugIn::CPlugIn {
    public:
        euroscope_mqtt();
        ~euroscope_mqtt();

        void DisplayMessage(const std::string &message, const std::string &sender = "euroscope-mqtt");
        
        void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) override;
        void SendMQTTCallsign(const std::string& callsign);
    };
}

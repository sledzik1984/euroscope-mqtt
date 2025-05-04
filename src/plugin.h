#pragma once

#pragma warning(push, 0)
#include "EuroScopePlugIn.h"
#pragma warning(pop)

#include <string>

namespace euroscope_mqtt
{
    class euroscope_mqtt : public EuroScopePlugIn::CPlugIn
    {
    public:
        euroscope_mqtt();
        ~euroscope_mqtt();

        void DisplayMessage(const std::string &message,
                            const std::string &sender = "euroscope-mqtt");
    };
}

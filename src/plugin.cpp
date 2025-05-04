#include "plugin.h"
#include "Version.h"
#include <string>

namespace euroscope_mqtt {

    euroscope_mqtt::euroscope_mqtt()
        : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_AUTHOR, PLUGIN_LICENSE) {
        // Wyświetlanie komunikatu w EuroScope na załadowanie pluginu
        DisplayMessage("Version " + std::string(PLUGIN_VERSION) + " loaded", "Initialization");

    }

    euroscope_mqtt::~euroscope_mqtt() {
        // Destruktor, pusty jeśli nic nie trzeba posprzątać
    }

    void euroscope_mqtt::DisplayMessage(const std::string &message, const std::string &sender) {
        // Wyświetlanie wiadomości w interfejsie EuroScope
        DisplayUserMessage(PLUGIN_NAME, sender.c_str(), message.c_str(), true, false, false, false, false);
    }



}

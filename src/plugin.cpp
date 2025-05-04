#include "plugin.h"
#include "Version.h"
#include <mosquitto.h>

namespace euroscope_mqtt
{
    void SendTestMessage()
    {
        const char* broker = "localhost";
        const int port = 1883;
        const char* topic = "euroscope/test";
        const char* message = "Test message from euroscope-mqtt";

        // Inicjalizacja klienta Mosquitto
        mosquitto_lib_init();
        mosquitto* mosq = mosquitto_new(NULL, true, NULL);
        if (!mosq) {
            std::cerr << "Error: Unable to create Mosquitto client" << std::endl;
            return;
        }

        // Połączenie z brokerem
        if (mosquitto_connect(mosq, broker, port, 60)) {
            std::cerr << "Error: Unable to connect to broker" << std::endl;
            mosquitto_destroy(mosq);
            mosquitto_lib_cleanup();
            return;
        }

        // Wysłanie wiadomości
        mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false);

        // Rozłączenie
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    }

    euroscope_mqtt::euroscope_mqtt()
        : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_AUTHOR, PLUGIN_LICENSE)
    {
        DisplayMessage("Version " + std::string(PLUGIN_VERSION) + " loaded", "Initialisation");

        // Wysyłanie testowej wiadomości MQTT
        SendTestMessage();
    }

    euroscope_mqtt::~euroscope_mqtt()
    {
    }

    void euroscope_mqtt::DisplayMessage(const std::string &message, const std::string &sender)
    {
        DisplayUserMessage(PLUGIN_NAME, sender.c_str(), message.c_str(), true, false, false, false, false);
    }
}

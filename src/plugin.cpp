#include "plugin.h"
#include "Version.h"
#include "mqtt.h" // Dołącz plik nagłówkowy MQTT-C

namespace euroscope_mqtt
{
    void SendTestMessage()
    {
        const char *address = "tcp://localhost:1883";
        const char *client_id = "euroscope-mqtt-test-client";
        const char *topic = "euroscope/test";
        const char *payload = "Test message from euroscope-mqtt";

        mqtt_client_t client;
        mqtt_init(&client, address, client_id, NULL, NULL);

        // Łączenie z brokerem MQTT
        if (mqtt_connect(&client) == MQTT_OK)
        {
            // Wysłanie wiadomości
            mqtt_publish(&client, topic, payload, strlen(payload), MQTT_QOS0);
            mqtt_disconnect(&client);
        }
        else
        {
            std::cerr << "Błąd połączenia z brokerem MQTT!" << std::endl;
        }
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

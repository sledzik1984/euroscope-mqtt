#include "plugin.h"
#include "Version.h"
#include <mqtt/client.h>

namespace euroscope_mqtt
{
    void SendTestMessage()
    {
        const std::string address = "tcp://localhost:1883";
        const std::string client_id = "euroscope-mqtt-test-client";
        const std::string topic = "euroscope/test";
        const std::string payload = "Test message from euroscope-mqtt";

        try
        {
            mqtt::client client(address, client_id, mqtt::create_options(MQTTVERSION_3_1_1));
            mqtt::connect_options conn_opts;
            conn_opts.set_clean_session(true);

            client.connect(conn_opts);
            auto msg = mqtt::make_message(topic, payload);
            msg->set_qos(0);
            msg->set_retained(false);
            client.publish(msg);
            client.disconnect();
        }
        catch (const mqtt::exception &e)
        {
            std::cerr << "[MQTT] Błąd: " << e.what() << std::endl;
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

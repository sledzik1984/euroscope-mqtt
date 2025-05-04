#include "plugin.h"
#include "Version.h"
#include <mqtt/client.h>
#include <fstream>
#include <sstream>

namespace euroscope_mqtt
{
    void SendTestMessage()
    {
        // Wartości domyślne
        std::string address = "tcp://cma.pl:1883";
        std::string topic = "euroscope/test";
        std::string username, password;

        // Wczytanie z pliku
        std::ifstream creds("mqtt_credentials.txt");
        if (!creds)
        {
            std::cerr << "[MQTT] Nie znaleziono pliku mqtt_credentials.txt" << std::endl;
            return;
        }

        std::string line;
        while (std::getline(creds, line))
        {
            if (line.find("username=") == 0)
                username = line.substr(9);
            else if (line.find("password=") == 0)
                password = line.substr(9);
            else if (line.find("broker=") == 0)
                address = line.substr(7);
            else if (line.find("topic=") == 0)
                topic = line.substr(6);
        }

        const std::string client_id = "euroscope-mqtt-test-client";
        const std::string payload = "Test message from euroscope-mqtt";

        try
        {
            mqtt::client client(address, client_id, mqtt::create_options(MQTTVERSION_3_1_1));
            mqtt::connect_options conn_opts;
            conn_opts.set_clean_session(true);
            conn_opts.set_user_name(username);
            conn_opts.set_password(password);

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

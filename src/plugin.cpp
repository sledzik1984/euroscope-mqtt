#include "plugin.h"
#include "Version.h"
#include "mqtt.h"  // Include the MQTT-C client header
#include <iostream>  // Add this header to use std::cout

namespace euroscope_mqtt {

    // Callback function to handle the message publication
    void publish_callback(void** unused, struct mqtt_response_publish *published) {
        std::string payload((char*)published->application_message, published->application_message_size);
        std::cout << "Received Message: " << payload << std::endl;  // Use std::cout here
    }

    void SendTestMessage() {
        const std::string address = "tcp://localhost:1883";  // Broker address
        const std::string client_id = "euroscope-mqtt-test-client";  // Client ID
        const std::string topic = "euroscope/test";  // Topic for sending message
        const std::string payload = "Test message from euroscope-mqtt";  // Message payload

        // Setup a client
        struct mqtt_client client;
        uint8_t sendbuf[2048];  // Send buffer large enough to hold multiple MQTT messages
        uint8_t recvbuf[1024];  // Receive buffer large enough for any expected MQTT message

        // Initialize the MQTT client
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);  // Initialize socket for communication with broker
        if (sockfd < 0) {
            std::cerr << "[MQTT] Failed to create socket!" << std::endl;
            return;
        }

        // Initialize the client with necessary parameters
        mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);

        // Connect to the MQTT broker
        if (mqtt_connect(&client, address.c_str(), client_id.c_str(), NULL, NULL, 60, 0) != 0) {
            std::cerr << "[MQTT] Connection failed!" << std::endl;
            return;
        }

        // Publish a message
        if (mqtt_publish(&client, topic.c_str(), payload.c_str(), payload.size(), MQTT_QOS0, false) != 0) {
            std::cerr << "[MQTT] Failed to send message!" << std::endl;
            return;
        }

        // Disconnect from the broker
        mqtt_disconnect(&client);
    }

    euroscope_mqtt::euroscope_mqtt()
        : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_AUTHOR, PLUGIN_LICENSE) {
        // Display message in EuroScope on plugin load
        DisplayMessage("Version " + std::string(PLUGIN_VERSION) + " loaded", "Initialization");

        // Call the function to send a test message
        SendTestMessage();
    }

    euroscope_mqtt::~euroscope_mqtt() {
        // Destructor (can be left empty if nothing to clean up)
    }

    void euroscope_mqtt::DisplayMessage(const std::string &message, const std::string &sender) {
        // Display the message in the EuroScope user interface
        DisplayUserMessage(PLUGIN_NAME, sender.c_str(), message.c_str(), true, false, false, false, false);
    }

    // Optional: Define any other necessary methods here for additional features

}

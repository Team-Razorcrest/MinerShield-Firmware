#include "node.h"
#include <WiFi.h>
#include "constants.h"
#include "Utils/utils.h"

#ifdef GATEWAY_DEVICE

WiFiServer server(80);

status_t init_node()
{
    Serial.begin(115200);
    bool res = WiFi.mode(WIFI_AP_STA);
    if(!res) return ERROR;
    return OKAY; 
}

status_t connect()
{
    Serial.println("Connecting to WiFi");
    bool res  = WiFi.softAP("SSID", "PASSWORD");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    return OKAY;
}

status_t disconnect()
{
    Serial.println("Disconnecting from WiFi");
    WiFi.disconnect();
}

status_t receive()
{
    Serial.println("Waiting for client");
    while (1)
    {
        WiFiClient client = server.available();
        if (client)
        {
            Serial.println("New Client");
            while (client.connected())
            {
                if (client.available())
                {
                    String line = client.readStringUntil('\r');
                    Serial.print(line);
                }
            }
            client.stop();
            Serial.println("Client disconnected");
        }
    }
}

#endif // GATEWAY_DEVICE

#ifdef HELMENT_DEVICE
status_t init_node()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}

status_t connect()
{
    WiFi.begin("SSID", "PASSWORD");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }
    return OKAY;
}

status_t send()
{
    WiFiClient client;
  // Connect to the server (AP) on the other ESP32
  if (client.connect(WiFi.gatewayIP(), 80)) {
    Serial.println("Connected to server");
    client.println("Weda httoooo!");  // Send data to the server
  } else {
    Serial.println("Connection to server failed");
    show_error();
  }
}

#endif // GATEWAY_DEVICE
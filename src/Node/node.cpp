#include "node.h"
#include "wifi.h"
#include "constants.h"

#ifdef GATEWAY_DEVICE

#endif // GATEWAY_DEVICE
#include <Utils/utils.h>

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
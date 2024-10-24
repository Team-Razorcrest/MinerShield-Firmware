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
    if (!res)
        return ERROR;
    return OKAY;
}

status_t connect()
{
    Serial.println("Connecting to WiFi");
    bool res = WiFi.softAP("SSID", "PASSWORD");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.begin(); // Start the server
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
                    String line = client.readStringUntil('\n');
                    Serial.print(line);
                }
            }
            Serial.println("Client disconnected");
        }
        client.stop();
    }
}

#endif // GATEWAY_DEVICE

#ifdef HELMENT_DEVICE

WiFiClient client;

status_t init_node()
{
    Serial.begin(115200);
    bool res = WiFi.mode(WIFI_STA);
    if (!res)
        return ERROR;
    return OKAY;
    res = WiFi.disconnect();
    if (!res)
        return ERROR;
    return OKAY;
}

status_t connect()
{
    Serial.println("Connecting to WiFi");
    bool res = WiFi.begin("SSID", "PASSWORD");
    if (!res)
        return ERROR;
    int retries = MAX_RETRIES;
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
        if (retries == 0)
            return ERROR;
        retries--;
    }
    Serial.println("\nConnected to WiFi");
    int res_ = client.connect(WiFi.gatewayIP(), 80);
    if (res_ < 0)
    {
        Serial.println("Connection to server failed");
        show_error();
        return ERROR;
    }
    Serial.println("Connected to server");
    client.println("Hello from Helment");
    return OKAY;
}

status_t send()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        connect();
        Serial.println("Connecting to WiFi");
    }

    client.println("Data from Helment");
    return OKAY;
}

#endif // GATEWAY_DEVICE
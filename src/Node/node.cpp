#include "node.h"
#include <WiFi.h>
#include "constants.h"
#include "Utils/utils.h"
#include <mbedtls/sha256.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "certificate.h"

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

const char *mqtt_username = "minershield";
const char *mqtt_password = "ms2024";

String _generatePassword(const String &ssid, int length = 12)
{
    // buffer to hold the hashed result
    unsigned char hash[32];

    // Perform SHA-256 hashing on the SSID
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts(&sha256_ctx, 0); // 0 = SHA-256 (not SHA-224)
    mbedtls_sha256_update(&sha256_ctx, (const unsigned char *)ssid.c_str(), ssid.length());
    mbedtls_sha256_finish(&sha256_ctx, hash);
    mbedtls_sha256_free(&sha256_ctx);

    // Convert the hash result to a readable string (hex or base64 style)
    String password = "";
    for (int i = 0; i < length; i++)
    {
        // Use modulo to stay within the length of the hash (32 bytes)
        password += String(hash[i % 32], HEX);
    }

    // Ensure password has the required length
    return password.substring(0, length);
}

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
    String ssid = "Razor-Crust-Gateway";
    String password = _generatePassword(ssid);
    Serial.println(password);
    bool res = WiFi.softAP(ssid, password.c_str());
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.begin(); // Start the server
    return OKAY;
}

status_t station_connect()
{
    bool res = WiFi.mode(WIFI_STA);
    if (!res)
        return ERROR;
    Serial.println("Connecting to WiFi");
    String ssid = "Redmi12";
    String password = "1234567890";
    res = WiFi.begin(ssid.c_str(), password.c_str());
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
    Serial.println("\nConnected to WiFi : " + ssid);
    secureClient.setCACert(root_ca);
    return OKAY;
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

status_t mqtt_connect()
{
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(callback);
    Serial.println("Connecting to MQTT Broker");
    return OKAY;
}

status_t mqtt_reconnect()
{
    while (!mqttClient.connected())
    {
        if (mqttClient.connect(MQTT_CLIENT_ID, mqtt_username, mqtt_password))
        {
            mqttClient.subscribe(MQTT_TOPIC);
        }
    }
    Serial.println("Connected to MQTT Broker");
    return OKAY;
}

status_t mqtt_msg_publish(String msg)
{
    if (!mqttClient.connected())
    {
        mqtt_reconnect();
    }
    bool res = mqttClient.publish(MQTT_TOPIC, msg.c_str());
    Serial.println("Publishing message");
    mqttClient.loop();
    secureClient.stop();
    return OKAY;
}

status_t send()
{
    if (!mqttClient.connected())
    {
        mqtt_reconnect();
    }
    return OKAY;
}

status_t disconnect()
{
    Serial.println("Disconnecting from WiFi");
    WiFi.disconnect();
}

status_t decode_msg(String msg, JsonDocument *dataJson)
{
    DeserializationError error = deserializeJson(*dataJson, msg);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return ERROR;
    }
    return OKAY;
}

status_t receive(String *data)
{
    unsigned long timeout_ms = 5000;  // Timeout in milliseconds
    Serial.println("Waiting for client");
    bool data_received = false;
    unsigned long start_time = millis();  // Timeout tracking

    while (1)  // Add a timeout condition
    {
        WiFiClient client = server.available();
        if (client)
        {
            Serial.println("New Client");
            while (client.connected())
            {
                if (client.available())
                {
                    Serial.println("Data received");
                    *data = client.readStringUntil('\n');
                    data_received = true;
                    break;
                }
            }
            Serial.println("Client disconnected");
            client.stop();  // Close connection properly
            if (data_received) break;
        }
    }

    if (!data_received) {
        Serial.println("Timeout: No data received");
        return ERROR;  // Return a timeout status if no data was received
    }

    return OKAY;
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
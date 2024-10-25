#include "node.h"
#include <WiFi.h>
#include "constants.h"
#include "Utils/utils.h"
#include <mbedtls/sha256.h>
#include <ArduinoJson.h>

#ifdef GATEWAY_DEVICE

#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "certificate.h"

#endif // GATEWAY_DEVICE

#ifdef GATEWAY_DEVICE

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

const char *mqtt_username = "minershield";
const char *mqtt_password = "ms2024";

#endif // GATEWAY_DEVICE

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
    unsigned long timeout_ms = 5000; // Timeout in milliseconds
    Serial.println("Waiting for client");
    bool data_received = false;
    unsigned long start_time = millis(); // Timeout tracking

    while (1) // Add a timeout condition
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
            client.stop(); // Close connection properly
            if (data_received)
                break;
        }
    }

    if (!data_received)
    {
        Serial.println("Timeout: No data received");
        return ERROR; // Return a timeout status if no data was received
    }

    return OKAY;
}

#endif // GATEWAY_DEVICE

#ifdef HELMET_DEVICE

WiFiClient client;

String *_get_available_networks(int *size)
{
    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    *size = n;
    if (n == 0)
    {
        Serial.println("No networks found.");
        return NULL; // No networks found, return NULL
    }
    else
    {
        // Allocate memory for the SSID list
        String *ssidList = new String[n];
        int *rssiList = new int[n]; // Use dynamic allocation to keep consistency

        // Populate the arrays with SSID and RSSI values
        for (int i = 0; i < n; i++)
        {
            ssidList[i] = WiFi.SSID(i);
            rssiList[i] = WiFi.RSSI(i);
        }

        // Sort by RSSI (signal strength)
        for (int i = 0; i < n - 1; i++)
        {
            for (int j = i + 1; j < n; j++)
            {
                if (rssiList[i] < rssiList[j])
                { // Sort in descending order
                    // Swap RSSI values
                    int tempRSSI = rssiList[i];
                    rssiList[i] = rssiList[j];
                    rssiList[j] = tempRSSI;

                    // Swap corresponding SSID values
                    String tempSSID = ssidList[i];
                    ssidList[i] = ssidList[j];
                    ssidList[j] = tempSSID;
                }
            }
        }

        // Clean up the RSSI list since we don't return it
        delete[] rssiList;

        return ssidList;
    }
}

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
    int size = 0;
    String *networks = _get_available_networks(&size);
    Serial.println("Size: " + (String)size);

    for (int i = 0; i < size; i++)
    {
        String SSID = networks[i];
        Serial.println("\nSSID " + (String)i + " " + SSID);
        String password = _generatePassword(SSID);
        Serial.println("password :" + password);
        bool res = WiFi.begin(SSID.c_str(), password.c_str());
        if (!res)
            return ERROR;
        int retries = MAX_RETRIES;
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(".");
            delay(500);
            if (retries == 0)
                break;
            retries--;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Connected to WiFi : " + SSID);
            break;
        }
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connection to WiFi failed");
        return ERROR;
    }

    int res_ = client.connect(WiFi.gatewayIP(), 80);
    if (res_ < 0)
    {
        Serial.println("Connection to server failed");
        show_error();
        return ERROR;
    }
    Serial.println("Connected to server");
    char dataJson[100];
    JsonDocument data;
    data["id"] = "AB3422H";

    int dht[2];
    readDHT(dht);

    data["temperature"] = dht[0];
    data["humidity"] = dht[1];
    data["methane"] = readMethane();
    data["fall_detection"] = 0;

    serializeJson(data, dataJson);
    Serial.println(dataJson);
    client.println(dataJson);
    return OKAY;
}

status_t send()
{
    char dataJson[100];
    JsonDocument data;
    data["id"] = "AB3422H";

    int dht[2];
    readDHT(dht);

    data["temperature"] = dht[0];
    data["humidity"] = dht[1];
    data["methane"] = readMethane();
    data["fall_detection"] = 0;

    serializeJson(data, dataJson);
    Serial.println(dataJson);
    client.println(dataJson);
    return OKAY;
}

#endif // HELMENT_DEVICE
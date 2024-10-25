#include "node.h"
#include <WiFi.h>
#include "constants.h"
#include "Utils/utils.h"
#include <mbedtls/sha256.h>
#include <ArduinoJson.h>

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

bool _isArrayEmpty(String arr[], int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i].length() > 0) {
            return false;  // At least one element is not empty
        }
    }
    return true;  // All elements are empty
}


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
            if (WiFi.SSID(i).substring(0, 5) == "Razor")
            {
                ssidList[i] = WiFi.SSID(i);
                rssiList[i] = WiFi.RSSI(i);
            } 
        }
        if (_isArrayEmpty(ssidList, n))
        {
            Serial.println("No Razor networks found.");
            ESP.restart();
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
        if (SSID.length() == 0)
            continue;
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
    data["fall_detection"] = readFallDetection()?1:0;

    serializeJson(data, dataJson);
    Serial.println(dataJson);
    client.println(dataJson);
    return OKAY;
}

status_t send()
{
    // char dataJson[100];
    // JsonDocument data;
    // data["id"] = "AB3422H";
    // data["type"] = "Data";
    // JsonObject devicedata = data.createNestedObject("data");

    // int dht[2];
    // readDHT(dht);

    // devicedata["temperature"] = dht[0];
    // devicedata["humidity"] = dht[1];
    // devicedata["methane"] = readMethane();
    // devicedata["fall_detection"] = 0;

    // serializeJson(data, dataJson);
    // client.println(dataJson);
    // Serial.println(dataJson);
    // return OKAY;
        char dataJson[100];
    JsonDocument data;
    data["id"] = "AB3422H";

    int dht[2];
    readDHT(dht);

    data["temperature"] = dht[0];
    data["humidity"] = dht[1];
    data["methane"] = readMethane();
    data["fall_detection"] = readFallDetection()?1:0;

    serializeJson(data, dataJson);
    Serial.println(dataJson);
    client.println(dataJson);
    return OKAY;
}

#endif // GATEWAY_DEVICE
#include "node.h"
#include <WiFi.h>
#include "constants.h"
#include "Utils/utils.h"
#include <mbedtls/sha256.h>

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
    String *networks = _get_available_networks();
    if (networks == NULL)
    {
        Serial.println("No networks found");
        return ERROR;
    }
    for (int i = 0; i <  sizeof(networks) / sizeof(networks[0]); i++)
    {
        String SSID = networks[i];
        Serial.println(SSID);
        String password = _generatePassword(SSID);
        bool res = WiFi.begin(SSID, password);
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

String *_get_available_networks()
{
    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
        Serial.println("No networks found.");
        return NULL;
    }
    else
    {
        // Create an array to hold the SSID and RSSI values
        String ssidList[n];
        int rssiList[n];

        // Get all SSID and RSSI values
        for (int i = 0; i < n; i++)
        {
            ssidList[i] = WiFi.SSID(i);
            rssiList[i] = WiFi.RSSI(i);
        }

        // Sort the networks by RSSI (signal strength)
        for (int i = 0; i < n - 1; i++)
        {
            for (int j = i + 1; j < n; j++)
            {
                if (rssiList[i] < rssiList[j])
                { // Sort in descending order (strongest signal first)
                    // Swap RSSI values
                    int tempRSSI = rssiList[i];
                    rssiList[i] = rssiList[j];
                    rssiList[j] = tempRSSI;

                    // Swap SSID names to match the RSSI swap
                    String tempSSID = ssidList[i];
                    ssidList[i] = ssidList[j];
                    ssidList[j] = tempSSID;
                }
            }
        }
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

#endif // GATEWAY_DEVICE
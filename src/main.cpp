#include <Arduino.h>
#include "Node/node.h"
#include "Utils/utils.h"
#include <ArduinoJson.h>

#ifdef GATEWAY_DEVICE

JsonDocument dataJson;
device_mode_t mode = ACCESS_POINT;

#endif // GATEWAY_DEVICE

void setup()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  init_error_mechanism();

#ifdef GATEWAY_DEVICE

  mode = ACCESS_POINT;

#endif // GATEWAY_DEVICE

#ifdef HELMET_DEVICE
  status_t status = init_node();
  if (status == ERROR)
  {
    show_error();
    ESP.restart();
  }

  status = connect();
  if (status == OKAY)
  {
    Serial.println("Connected to WiFi");
    digitalWrite(LED_GREEN, LOW);
  }
  else
  {
    show_error();
    ESP.restart();
  }

  digitalWrite(LED_RED, HIGH);
  status = send();
  if (status == ERROR)
  {
    show_error();
    ESP.restart();
  }
  delay(1000);
  digitalWrite(LED_RED, LOW);
  esp_sleep_enable_timer_wakeup(WAKEUP_INTERVAL * 60000000);
  esp_deep_sleep_start();
#endif
}

void loop()
{
#ifdef GATEWAY_DEVICE
  if (mode == ACCESS_POINT)
  {
    digitalWrite(LED_RED, HIGH);
    status_t status = init_node();
    if (status == ERROR)
    {
      show_error();
    }
    status = connect();
    if (status == OKAY)
    {
      Serial.println("Initialized as Access Point");
    }
    else
    {
      show_error();
    }
    String data;
    status = receive(&data);
    Serial.println(data);
    if (status == ERROR)
    {
      show_error();
    }
    else if (status == OKAY)
    {
      Serial.println("Received data");
    }
    status = decode_msg(data, &dataJson);
    if (status == ERROR)
    {
      show_error();
    }
    else if (status == OKAY)
    {
      Serial.println("Decoded data");
      Serial.println(dataJson["temperature"].as<float>());
      Serial.println(dataJson["humidity"].as<float>());
      Serial.println(dataJson["methane"].as<float>());
      Serial.println(dataJson["fall_detection"].as<float>());
    }
    mode = STATION;
    digitalWrite(LED_RED, LOW);
  }
  else if (mode == STATION)
  {
    digitalWrite(LED_GREEN, HIGH);
    Serial.println("Initialized as Station");
    status_t status = station_connect();
    if (status == ERROR)
    {
      show_error();
      ESP.restart();
    }
    Serial.println("Connected to WiFi");
    status = mqtt_connect();
    if (status == ERROR)
    {
      show_error();
    }
    status = mqtt_reconnect();
    if (status == ERROR)
    {
      show_error();
    }
    char msg[100];
    JsonDocument finalJson;
    JsonDocument nodeJson;
    nodeJson["temperature"] = dataJson["temperature"];
    nodeJson["humidity"] = dataJson["humidity"];
    nodeJson["methane"] = dataJson["methane"];
    nodeJson["fall_detection"] = dataJson["fall_detection"];
    finalJson["gatewayId"] = 1;
    JsonArray nodes = finalJson["nodes"].to<JsonArray>();
    nodes.add(nodeJson);
    serializeJson(finalJson, msg);
    Serial.println("serialized data");
    Serial.println(msg);
    status = mqtt_msg_publish(msg);
    if (status == ERROR)
    {
      show_error();
    }
    else if (status == OKAY)
    {
      Serial.println("Published data");
      mode = ACCESS_POINT;
    }
    digitalWrite(LED_GREEN, LOW);
  }

#endif // GATEWAY_DEVICE
}

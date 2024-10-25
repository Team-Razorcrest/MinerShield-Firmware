#include <Arduino.h>
#include "Node/node.h"
#include "Utils/utils.h"

void setup()
{
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  digitalWrite(LED_GREEN,HIGH);
  init_error_mechanism();
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
    digitalWrite(LED_GREEN,LOW);
  }
  else
  {
    show_error();
    ESP.restart();
  }
#ifdef GATEWAY_DEVICE
  status = receive();
#endif
#ifdef HELMENT_DEVICE
  digitalWrite(LED_RED,HIGH);
  status = send();
  if (status == ERROR)
  {
    show_error();
    ESP.restart();
  }
    delay(1000);
   digitalWrite(LED_RED,LOW);
  esp_sleep_enable_timer_wakeup(WAKEUP_INTERVAL * 60000000);
  esp_deep_sleep_start();
#endif
}

void loop()
{
}

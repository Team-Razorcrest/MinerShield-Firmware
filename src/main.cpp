#include <Arduino.h>
#include "Node/node.h"
#include "Utils/utils.h"

void setup()
{
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
  status = send();
  if (status == ERROR)
  {
    show_error();
    ESP.restart();
  }
  esp_sleep_enable_timer_wakeup(WAKEUP_INTERVAL * 60000000);
  esp_deep_sleep_start();
#endif
}

void loop()
{
}

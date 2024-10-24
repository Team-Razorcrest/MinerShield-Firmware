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
  }

  status = connect();
  if (status == OKAY)
  {
    Serial.println("Connected to WiFi");
  }
  else
  {
    show_error();
  }
#ifdef GATEWAY_DEVICE
  status = receive();
#endif
#ifdef HELMENT_DEVICE
  status = send();
#endif
}

void loop()
{
}

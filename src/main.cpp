#include <Arduino.h>
#include "constants.h"
#include "Node/node.h"
#include "Utils/utils.h"

void setup()
{
  init_error_mechanism();
  status_t status = init_node();
  if (status == ERROR)
  {
    show_error();
    ESP.deepSleep(0);
  }
  status = connect();
  if (status == ERROR)
  {
    show_error();
    ESP.deepSleep(0);
  }
  send();
}

void loop()
{
}

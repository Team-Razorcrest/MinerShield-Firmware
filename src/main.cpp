#include <Arduino.h>
#include "constants.h"
#include "Node/node.h"
#include "Utils/utils.h"


void setup() {
  init_error_mechanism();
  status_t status = init_node();
  if(status == OKAY) {
    Serial.println("Node initialized successfully");
  } else {
    show_error();
  }

}

void loop() {
}


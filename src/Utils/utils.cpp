#include "utils.h"

void init_error_mechanism() {
  pinMode(INBUILT_LED, OUTPUT);
}

status_t show_error() {
  digitalWrite(INBUILT_LED, HIGH);
  delay(1000);
  digitalWrite(INBUILT_LED, LOW);
  delay(1000);
  return OKAY;
}
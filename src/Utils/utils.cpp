#include "utils.h"

void init_error_mechanism() {
  pinMode(BUILTIN_LED, OUTPUT);
}

status_t show_error() {
  digitalWrite(BUILTIN_LED, HIGH);
  delay(1000);
  digitalWrite(BUILTIN_LED, LOW);
  delay(1000);
  return OKAY;
}
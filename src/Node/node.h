#ifndef __NODE_H__
#define __NODE_H__

#include "constants.h"
#include <Arduino.h>
#include <WiFi.h>
#include "structs.h"
#include <ArduinoJson.h>

status_t init_node();
status_t connect();
status_t disconnect();
status_t send();

#ifdef GATEWAY_DEVICE

status_t receive(String *data);
status_t decode_msg(String msg, JsonDocument *dataJson);
status_t station_connect();
status_t mqtt_connect();
status_t mqtt_msg_publish(String msg);
status_t mqtt_reconnect();

#endif // GATEWAY_DEVICE

#endif // __NODE_H__
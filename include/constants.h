#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#define GATEWAY_DEVICE // if device is a gateway
// #define HELMENT_DEVICE // if device is a helment

#ifdef GATEWAY_DEVICE

#define STATION_STATES 5

#define MQTT_SERVER "zf545ce4.ala.asia-southeast1.emqxsl.com"
#define MQTT_PORT 8883
#define MQTT_CLIENT_ID "ESP32-0101"

#define CO2_SENSOR_PIN 34
#define H2_SENSOR_POWER_PIN 35
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22



#define MQTT_TOPIC "gateway1"

#endif

#define MAX_RETRIES 10

#endif // __CONSTANTS_H__
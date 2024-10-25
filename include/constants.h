#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#define MAX_RETRIES 10
#define WAKEUP_INTERVAL 1

#define LED_RED 4
#define LED_GREEN 23


// #define GATEWAY_DEVICE // if device is a gateway
#define HELMET_DEVICE // if device is a helment

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

#ifdef HELMET_DEVICE

#define DHTPIN 32
#define MQ8_PIN 34

#endif // HELMENT_DEVICE

#endif // __CONSTANTS_H__
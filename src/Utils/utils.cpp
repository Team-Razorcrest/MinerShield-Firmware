#include "utils.h"
#include "DHT22.h"

#define DHTPIN 32
DHT22 dht22(DHTPIN);

void init_error_mechanism()
{
  pinMode(BUILTIN_LED, OUTPUT);
}

status_t show_error()
{
  digitalWrite(BUILTIN_LED, HIGH);
  delay(1000);
  digitalWrite(BUILTIN_LED, LOW);
  delay(1000);
  return OKAY;
}

void init_sensors()
{
}

void readDHT(int arr[2])
{
  int temp = (int)dht22.getTemperature();
  int humidity = (int)dht22.getHumidity();

  if (dht22.getLastError() != dht22.OK)
  {
    Serial.print("last error :");
    Serial.println(dht22.getLastError());
    arr[0] = (int)temp;
    arr[1] = (int)humidity;
    return;
  }
  arr[0] = (int)temp;
  arr[1] = (int)humidity;
}

int readMethane()
{
  return 100;
}

bool readFallDetection()
{
  return false;
}
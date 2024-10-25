#include "utils.h"
#include "DHT22.h"
#include "Adafruit_MPU6050.h"
#include "Adafruit_Sensor.h"
#include "Wire.h"

DHT22 dht22(DHTPIN);
Adafruit_MPU6050 mpu;

void init_error_mechanism()
{
  pinMode(BUILTIN_LED, OUTPUT);
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
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

int readAnalogSensor(int pin, int numSamples = 10)
{
  long total = 0;
  for (int i = 0; i < numSamples; i++)
  {
    total += analogRead(pin);
    delay(2);
  }

  return total / numSamples;
}

int readMethane()
{
  return readAnalogSensor(MQ8_PIN);
}

bool readFallDetection()
{
  return mpu.getMotionInterruptStatus();
}
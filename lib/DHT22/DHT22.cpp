#include "Arduino.h"
#include "DHT22.h"

uint64_t DHT22::getRawData(){
  readSensor();
  return _rawData;
}

String DHT22::getRawStrData(){
  String r="";
  for(int8_t i=39;i>=0;i--){
    if (bitRead(_rawData,i)) r +="1"; else r +="0";
    if (i==24 || i==8) r +=" ";
  }
  return r;
}

// Convert Celsius to Fahrenheit
float DHT22::convertCtoF(float c) { return c * 1.8 + 32; }
// Convert Fahrenheit to Celsius
float DHT22::convertFtoC(float f) { return (f - 32) * 0.55555; }

float DHT22::getHumidity(){
  if (readSensor() != OK) return -1;
  else return _h16bits/10.0;
}

// M = true : Celsius ; M = false : Fahrenheit
float DHT22::getTemperature(bool M){
  if (readSensor() != OK) return -273;
  float temperature = (bitRead(_t16bits, 15)) ? (_t16bits & 0x7FFF) / -10.0 : _t16bits / 10.0;
  return M ? temperature : convertCtoF(temperature);
}

// https://www.weather.gov/media/ffc/ta_htindx.PDF
float DHT22::getHeatIndex(float temperature, float percentHumidity, bool isCelsius){
  float hi;

  if (isCelsius)
    temperature = convertCtoF(temperature);

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) +
              (percentHumidity * 0.094));

  if (hi > 79)
  {
    hi = -42.379 + 2.04901523 * temperature + 10.14333127 * percentHumidity +
         -0.22475541 * temperature * percentHumidity +
         -0.00683783 * pow(temperature, 2) +
         -0.05481717 * pow(percentHumidity, 2) +
         0.00122874 * pow(temperature, 2) * percentHumidity +
         0.00085282 * temperature * pow(percentHumidity, 2) +
         -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if ((percentHumidity < 13) && (temperature >= 80.0) &&
        (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) *
            sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if ((percentHumidity > 85.0) && (temperature >= 80.0) &&
             (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  return isCelsius ? convertFtoC(hi) : hi;
}

uint8_t DHT22::getLastError(){
  return _lastError;
}

void DHT22::measureTimings(){
  //dht22 sampling rate ~0.5Hz
  if(!_firstStart && (millis()-_timer) < cSamplingTime){
    return;
  }
  _timer = millis();
  _firstStart=false;
  
  _timingBit0 = 0;
  _timingBit1 = 0;
  // sending START : Level LOW for at least 1ms
  pinMode(_pinData,OUTPUT);
  digitalWrite(_pinData,LOW);// specs 1-10ms mini
  delay(2);

  //Level HIGH and wait for sensor
  digitalWrite(_pinData,HIGH);
  pinMode(_pinData,INPUT);
  
  // sensor-> MCU
  uint32_t t = micros();
  uint32_t m = 0;
  while(digitalRead(_pinData)==1) {// wait for Level LOW specs 20-40µs
    if ( (micros()-t) > 60) return;    
  }
  t = micros();
  while(digitalRead(_pinData)==0) {// Level LOW 80µs
    m = micros()-t;
    if ( m > 100) return;    
  }
  _timing80L = m;
  t = micros();
  while(digitalRead(_pinData)==1) {// Level HIGH 80µs
    m = micros()-t;
    if ( m > 100) return;    
  }
  _timing80H = m;
  t = micros();
  while(digitalRead(_pinData)==0){//specs Level LOW 50µ$
    m = micros()-t;
    if (m > 60) return;    
  }
  _timing50 = m;
  t = micros();
  while(_timingBit0==0 || _timingBit1==0){
    while(digitalRead(_pinData)==1){//specs Level HIGH 26-28µs for "0" 70µs for "1"
      m = micros()-t;
      if (m > 100) return; 
    }
    if (m > 40){
      _timingBit1 = m;
    }
    else {
      _timingBit0 = m;
    } 
    t = micros();
    while(digitalRead(_pinData)==0){//wait for new bit
      m = micros()-t;
      if (m > 100) return; 
    }
    t = micros();
  }
}

uint8_t DHT22::readSensor(){
  //dht22 sampling rate ~0.5Hz
  if(!_firstStart && (millis()-_timer) < cSamplingTime){
    return OK;
  }
  _timer = millis();
  _firstStart = false;
  
  // sending START : Level LOW for at least 1ms
  pinMode(_pinData,OUTPUT);
  digitalWrite(_pinData,LOW);// specs 1-10ms mini
  delay(2);

  //Level HIGH and wait for sensor
  digitalWrite(_pinData,HIGH);
  pinMode(_pinData,INPUT);
  int32_t t = pulseIn(_pinData, HIGH, 250);
  if (t==0) return (_lastError = ERR_TIMING_80);

  _rawData = 0;
  //reading 40 bits
  for (uint8_t i=0;i<40;i++) {
    t = micros();
    while(digitalRead(_pinData)==0){//specs Level LOW 50µ$
      if ( (micros()-t) > _timing50+T) return (_lastError = ERR_TIMING_50) ;
    }
    delayMicroseconds(40);
    if (digitalRead(_pinData)==1) _rawData++;//specs Level HIGH 26-28µs for "0" 70µs for "1"
    if (i!=39) _rawData <<=1;
    t = micros();
    while(digitalRead(_pinData)==1){//if "1" wait for next 50µs Level LOW
      if ( (micros()-t) > _timingBit1) return (_lastError = ERR_TIMING_BITS) ;
    }
  }
  delayMicroseconds(10);
  pinMode(_pinData,OUTPUT);
  digitalWrite(_pinData,HIGH);
  _h16bits = _rawData>>24;
  _t16bits = _rawData>>8;
  _crc8bits = _rawData;
  if (computeCRC()) return (_lastError = OK);
  else return (_lastError = ERR_CRC);
}

String DHT22::debug(){
  String d = "### BEGIN DEDUG ###\nlook at datasheet for timing specs\n";
  measureTimings();
  
  d +="t_80L\tt_80H\tt_50\tt_Bit0\tt_Bit1\n";
  d += String(_timing80L) +"\t";
  d += String(_timing80H) +"\t";
  d += String(_timing50) +"\t";
  d += String(_timingBit0) +"\t";
  d += String(_timingBit1) +"\n";

  delay(cSamplingTime); // f is 0.5Hz
  uint8_t err = readSensor();
  d += "error : "+String(err)+"\n";  

  d += getRawStrData();
  d +="\nh\tt\tcrc\n";
  d += String(getHumidity(),1) +"\t";
  d += String(getTemperature(),1) +"\t";
  if (err != ERR_CRC) d+="TRUE"; else d+="FALSE";
  d+="\n### END DEDUG ###";
  return d;
}

bool DHT22::computeCRC(){
  uint8_t sum = highByte(_h16bits) + lowByte(_h16bits) + highByte(_t16bits) + lowByte(_t16bits) ; 
  return (sum == _crc8bits);
}

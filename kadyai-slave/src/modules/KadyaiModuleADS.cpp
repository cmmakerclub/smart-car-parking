#include "KadyaiModuleADS.h"
extern int temp;

extern char userEspnowSensorName[16];
extern CMMC_SENSOR_DATA_T userKadyaiData;

void KadyaiModuleADS::config(CMMC_System *os, AsyncWebServer* server) {
  static KadyaiModuleADS *that = this;
  sensor1 = new CMMC_BME280();
  sensor1->setup();
  sensor1->every(1);
  sensor1->onData([&](void *d, size_t len) {
    memcpy(&data1, d, len);
    Serial.printf("ON SENSOR DATA.. at %lums\r\n", millis());
    Serial.printf("== temp=%lu, humid=%lu, pressure=%lu\r\n", data1.field1, data1.field2, data1.field3);
  });
}

enum SENSOR_TYPE { PH=1, MOISTURE }; 
void KadyaiModuleADS::_read_sensor() {
  int moistureValue, phValue, batteryValue;
  int a0Val;
  /* battery */
  Serial.printf("Reading Battery.. (at %lums)\r\n", millis()); 
  /* pH */
  batteryValue = analogRead(A0) * 0.0051724137931034f * 100;
  int16_t adc0, adc1, adc2, adc3;
  adc0 = ads->readADC_SingleEnded(0);
  adc1 = ads->readADC_SingleEnded(1);
  adc2 = ads->readADC_SingleEnded(2);
  adc3 = ads->readADC_SingleEnded(3);
  Serial.print("AIN0: "); Serial.println(adc0);
  Serial.print("AIN1: "); Serial.println(adc1);
  Serial.print("AIN2: "); Serial.println(adc2);
  Serial.print("AIN3: "); Serial.println(adc3); 
  float phFloat =0;
  float moistureFloat =0;
  phFloat = 8.0-abs((adc1*0.0078125f)*0.0105f);
  moistureFloat = 1+(abs(adc0*0.0078125f)*0.04f);
  /* Moisture */
  a0Val = analogRead(A0);
  // moistureValue = ((a0Val * 0.035f) + 1) * 100; 
  moistureValue = moistureFloat * 100;
  phValue = phFloat * 100;
  Serial.printf("battery=%d, moisture=%d, pH=%d\r\n", batteryValue, moistureValue, phValue); 
  data1.battery = batteryValue; 
  data1.field1 = data1.field1; /* temp */
  data1.field2 = data1.field2; /* humid */
  data1.field4 = moistureValue;
  data1.field5 = data1.field3; /* pressure */
  data1.field3 = phValue;

  data1.field5 = adc0;
  data1.field6 = adc0;
  data1.field7 = adc1;
  data1.field8 = adc2;
  data1.field9 = adc3;
  // data1.field6 = data1.field6; /* field6 */
  // data1.field7 = data1.field7; /* field7 */
  // data1.field8 = data1.field8; /* field8 */
  // data1.field9 = data1.field9; /* field9 */
  data1.ms = millis();
  strcpy(data1.sensorName, userEspnowSensorName);
  data1.nameLen = strlen(data1.sensorName); 
  data1.sum = CMMC::checksum((uint8_t*) &data1, sizeof(data1) - sizeof(data1.sum)); 
  memcpy(&userKadyaiData, &data1, sizeof(data1));
  CMMC::dump((u8*) &userKadyaiData, sizeof(userKadyaiData));
}

void KadyaiModuleADS::configLoop() { 
  yield(); 
}

void KadyaiModuleADS::setup() {
  ads = new Adafruit_ADS1115(0x48);
  ads->setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV 
  ads->begin();
  sensor1->read();
  _read_sensor();
}

void KadyaiModuleADS::loop() { 
  yield();
}

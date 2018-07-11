#ifndef CMMC_KADYAI_ADS_MODULE_H
#define CMMC_KADYAI_ADS_MODULE_H 

#define CMMC_USE_ALIAS

#include <CMMC_Module.h>
#include <CMMC_Sensor.h>
#include <CMMC_Utils.h>
#include <CMMC_BME280.hpp> 
#include <Wire.h>
#include <Adafruit_ADS1015.h> 

class KadyaiModuleADS: public CMMC_Module {
  public:
    void config(CMMC_System *os, AsyncWebServer* server); 
    void setup();
    void loop(); 
    void configLoop();
  protected:
  private:
    void _read_sensor();
    CMMC_SENSOR_DATA_T data1;
    CMMC_Sensor *sensor1;
    Adafruit_ADS1115 *ads;
};

#endif
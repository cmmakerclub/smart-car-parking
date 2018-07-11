
#include <CMMC_Legend.h>
#include <modules/KadyaiModuleADS.h>
#include <modules/ESPNowModule.h>
#include <CMMC_Sensor.h> 

CMMC_Legend os;
CMMC_SENSOR_DATA_T userKadyaiData;
char userEspnowSensorName[16];

void setup()
{
  delay(10);
  Wire.begin(4, 5);
  os.addModule(new KadyaiModuleADS());
  os.addModule(new ESPNowModule());
  os.setup();
  Serial.printf("APP VERSION: %s\r\n", LEGEND_APP_VERSION);
}

void loop()
{
  os.run();
}
#include <Arduino.h> 
#include <esp_now.h>
#include <WiFi.h>
#include <CMMC_NB_IoT.h>
#include "data_type.h"

HardwareSerial Serial1(2);
CMMC_PACKET_T pArr[30];
int pArrIdx = 0;
char* espnowMsg[300]; 

#define rxPin 15
#define txPin 18 
#define LED_PIN 

// SoftwareSerial swSerial(rxPin, txPin);
CMMC_NB_IoT nb(&Serial1);
uint32_t counter = 0; 
uint32_t sentCnt = 0; 

void str2Hex(const char* text, char* buffer);
void toHexString(const uint8_t array[], size_t len, char buffer[]);

uint8_t currentSleepTimeMinuteByte = 2; 
bool dirty = false;
bool isNbConnected = false;
String token = "b98ce7b0-185b-11e8-8630-xxxxxxxxxxx5";
char tokenHex[100];
uint32_t prev;
// uint8_t remoteMac[6] = {0x2e, 0x3a, 0xe8, 0x12, 0xbe, 0x92};
esp_now_peer_info_t slave;
void setup() {
  bzero(&slave, sizeof(slave));
  Serial.begin(57600);
  Serial1.begin(9600);
  WiFi.disconnect();
  delay(20);
  WiFi.mode(WIFI_AP_STA);
  delay(50);
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  str2Hex(token.c_str(), tokenHex);
  Serial.println(tokenHex);
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
  delay(20);
    nb.setDebugStream(&Serial);

  nb.onDeviceReboot([]() {
    Serial.println(F("[user] Device rebooted."));
    // nb.queryDeviceInfo();
    // delay(1000);
  }); nb.onDeviceReady([]() {
    Serial.println("[user] Device Ready!");
  });

  nb.onDeviceInfo([](CMMC_NB_IoT::DeviceInfo device) {
    Serial.print(F("# Module IMEI-->  "));
    Serial.println(device.imei);
    Serial.print(F("# Firmware ver-->  "));
    Serial.println(device.firmware);
    Serial.print(F("# IMSI SIM-->  "));
    Serial.println(device.imsi);
  });

  nb.onMessageArrived([](char *text, size_t len, uint8_t socketId, char* ip, uint16_t port) {
    char buffer[100];
    sprintf(buffer, "++ [recv:] socketId=%u, ip=%s, port=%u, len=%d bytes (%lums)", socketId, ip, port, len, millis());
    Serial.println(buffer);
  });

  nb.onConnecting([]() {
    Serial.println("Connecting to NB-IoT...");
    delay(500);
  });

  nb.onConnected([]() {
    Serial.print("[user] NB-IoT Network connected at (");
    Serial.print(millis());
    Serial.println("ms)");
    Serial.println(nb.createUdpSocket("103.20.205.85", 5683, UDPConfig::ENABLE_RECV));
    Serial.println(nb.createUdpSocket("103.212.181.167", 55566, UDPConfig::ENABLE_RECV));
    isNbConnected = 1;
    delay(1000);
  });

  nb.rebootModule();

  esp_now_register_send_cb([&] (const uint8_t *mac_addr, esp_now_send_status_t status) {
    sentCnt++;
  });

  esp_now_register_recv_cb([&](const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    memcpy(&slave.peer_addr, mac_addr, 6);
    CMMC_PACKET_T wrapped;
    CMMC_SENSOR_DATA_T packet;
    memcpy(&packet, data, sizeof(packet));
    memcpy(&wrapped.data, &packet, sizeof(packet));
    wrapped.ms = millis();
    pArr[pArrIdx] = wrapped;
    pArrIdx = (pArrIdx + 1) % 30;
    esp_now_send(mac_addr, &currentSleepTimeMinuteByte, 1);
    wrapped.sleepTime = currentSleepTimeMinuteByte;
    
    for(int i =0 ; i < sizeof(wrapped); i++) {
      Serial.printf("%02x", ((uint8_t*)&wrapped)[i]); 
    }
    Serial.println(); 

    esp_err_t addStatus = esp_now_add_peer(&slave);
    // esp_err_t result = esp_now_send(mac_addr, data, data_len);
    counter++;
  });

  prev = millis();

}

void loop() {
  nb.loop();
      if ( (pArrIdx > 0) && (isNbConnected)) {
      char buffer[500];
      char b[500];
      Serial.printf("pArrIdx = %d\r\n", pArrIdx);
      for (int i = pArrIdx - 1; i >= 0; i--) {
        Serial.printf("reading idx = %d\r\n", i);
        toHexString((uint8_t*)  &pArr[i], sizeof(CMMC_PACKET_T), (char*)espnowMsg);
        sprintf(b, "{\"payload\": \"%s\"}", espnowMsg);
        str2Hex(b, buffer);
        String p3 = "";
        p3 += String("40");
        p3 += String("020579");
        p3 += String("b5");
        p3 += String("4e42496f54"); // NB-IoT
        p3 += String("0d");
        p3 += String("17");
        p3 +=  String(tokenHex);
        p3 += String("ff");
        p3 += String(buffer);
        int rt = 0;
        while (true) {
          if (nb.sendMessageHex(p3.c_str(), 0)) {
            Serial.println(">> [ais] socket0: send ok.");
            pArrIdx--;
            break;
          }
          else {
            Serial.println(">> [ais] socket0: send failed.");
            if (++rt > 5) {
              break;
            }
          }
        }
      }
      prev = millis();
    }
  // if (millis() - prev > 1000) {
  //   prev = millis();
  //   Serial.printf("rate: recv=%lu, sent=%lu hz\r", counter, sentCnt);
  //   counter = 0;
  //   sentCnt = 0;
  // }
}



void str2Hex(const char* text, char* buffer) {
  size_t len = strlen(text);
  for (int i = 0 ; i < len; i++) {
    sprintf(buffer + i * 2, "%02x", text[i]);
  }
}

void toHexString(const uint8_t array[], size_t len, char buffer[]) {
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}
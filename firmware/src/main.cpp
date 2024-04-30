#include <Arduino.h>
#include <IoTT_LocoNetHBESP32.h> 
#include <IoTT_lbServer.h>

#include "lbWifi.h"
#include "WebServer.h"
#include "lbLcd.h"
#include "loconet.h"




LocoNetESPSerial lnSerial(2); //true is inverted signals
IoTT_LBServer * lbServer=NULL;
IoTT_DigitraxBuffers * digitraxBuffer = NULL;
std::vector<wsClientInfo> globalClients; 
String deviceName="Esp32LB";


//used for performance analysis
#ifdef showPerformance
  uint16_t loopCtr = 0;
  uint32_t myTimer = millis() + 1000;
#endif



void setup() {
  // put your setup code here, to run once:
  pinMode(BTN,INPUT_PULLUP);
  pinMode(PIN_CFG,INPUT);
  pinMode(PIN_LED,OUTPUT);
  pinMode(LED_R,OUTPUT);
  pinMode(LED_G,OUTPUT);
  pinMode(LED_B,OUTPUT);
  digitalWrite(LED_R,0);
  digitalWrite(LED_G,0);
  digitalWrite(LED_B,0); 
  initLcd();

  Serial.begin(115200); 
  lnSerial.setBusyLED(LED_R);
  lnSerial.begin(pinRx,pinTx,false,true);
  pinMode(LED_R,OUTPUT);
  digitalWrite(LED_R,1);
  digitalWrite(LED_R,0);

  //while (!Serial);
  initWifi();  
  digitraxBuffer = new IoTT_DigitraxBuffers(sendMsg);
  lbServer=new IoTT_LBServer();
  lbServer->initLBServer(true);
  lbServer->initMDNS();
  MDNS.addService("http","tcp",80);
  MDNS.addService("locobuffer","tcp",1234);
  delay(1000);
  
  lbServer->startServer();
  initWebServer();
  Serial.println("Init Done");

  randomSeed((uint32_t)ESP.getEfuseMac()); //initialize random generator with MAC
}

void loop() {
  // put your main code here, to run repeatedly:

#ifdef showPerformance
  loopCtr++;
  if (millis() > myTimer)
  {
    Serial.printf("Timer Loop: %i\n", loopCtr);
    loopCtr = 0;
    myTimer += 1000;
  }
#endif
  lnSerial.processLoop();
  lbServer->processLoop();
  LCD_update_loop();
  yield();
}

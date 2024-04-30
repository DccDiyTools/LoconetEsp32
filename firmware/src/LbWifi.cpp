#include <Arduino.h>
#include "lbWifi.h"
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "WebServer.h"
#include "lbLcd.h"

String ssid     = "LocoBuffer-AP";
String password = "12345678";
String hostname = "LocoBuffer-AP";
bool staticIp=false;
IPAddress ip;
IPAddress nm;
IPAddress gw;
IPAddress dns[2];


bool isAP=false;
bool initCP=false;

void blinkTimes(int blinks, int onDelay, int offDelay){
  for(int i=0;i<blinks;i++){
    digitalWrite(LED_G,1);
    digitalWrite(LED_B,1);
    delay(onDelay);
    digitalWrite(LED_G,0);
    digitalWrite(LED_B,0);   
    delay(offDelay);
  }
}

uint32_t readFileToBuffer(String fileName, char * thisBuffer, uint32_t maxSize)
{
  uint32_t bytesRead = 0;
//  Serial.printf("Trying to read File %s\n", &fileName[0]);
  if (SPIFFS.exists(fileName))
  {
    File dataFile = SPIFFS.open(fileName, "r");
    if (dataFile)
    {
      if (dataFile.size() < maxSize)
      {
        bytesRead = dataFile.size();
        dataFile.read((uint8_t*)thisBuffer, bytesRead);
        thisBuffer[bytesRead] = '\0'; 
      }
      dataFile.close();
    }
    else 
      Serial.printf("Can't open %s \n", &fileName[0]);
  } 
  else 
    Serial.printf("File %s not found\n", &fileName[0]);
  return bytesRead;
}


DynamicJsonDocument * getDocument(char* buffer, const char* file){
  if(SPIFFS.begin()){
    uint32_t jsonData = readFileToBuffer(file, buffer, BUFFER_SIZE);
    if (jsonData > 0){
      uint16_t docSize = 4096 * (trunc((3 * jsonData) / 4096) + 1);
      //Serial.println(buffer);
      //Serial.printf("Size: %i Doc Size: %i\n", jsonData, docSize);
      DynamicJsonDocument * thisDoc = new DynamicJsonDocument(docSize);
      DeserializationError error;
      error = deserializeJson(*thisDoc, buffer);
      if(error) {
        Serial.println("Deserialization error");
        return NULL;
      }
      return thisDoc;
    
    }else{
      Serial.println("error reading wifi.json");
      return NULL;
    }
  }else{
    Serial.println(F("Not able to open SPIFFS"));
    return NULL;
  }
}

bool loadConfigData()
{
  Serial.println("Reading wifi.json");
  char* buffer= (char*) malloc(BUFFER_SIZE);
  DynamicJsonDocument* doc = getDocument(buffer,"/wifi.json");
  bool res =false;
  if(doc!=NULL){
    res= doc->containsKey("mode");
    if(res){
      String mode = (*doc)["mode"];
      Serial.printf("MODE: %s\n",mode.c_str());
      if(mode == "AP"){
        isAP=true;
      }else if(mode == "STA"){
        isAP=false;
      }else{
        res=false;
      }
    }
    res = res && doc->containsKey("SSID");
    if(res){
      String _ssid=(*doc)["SSID"];
      Serial.printf("SSID: %s\n", _ssid.c_str());
      if(_ssid !=""){
        ssid=_ssid;
      }else{
        res=false;
      }
    }
    res = res && doc->containsKey("password");
    if(res){
      String _password=(*doc)["password"];
      if(isAP)Serial.printf("password: %s\n", _password.c_str());
      if(_password !=""){
        password=_password;
      }else{
        res=false;
      }
    }
    res = res && doc->containsKey("webPass");
    if(res){
      String _webPass=(*doc)["webPass"];
      Serial.printf("webPass: %s\n", _webPass.c_str());
      if(_webPass !=""){
        webPass=_webPass;
      }else{
        res=false;
      }
    }
    res=res && doc->containsKey("hostname");
    if(res){
      String _hostname=(*doc)["hostname"];
      Serial.printf("hostname: %s\n", _hostname.c_str());
      if(_hostname !=""){
        hostname=_hostname;
      }else{
        res=false;
      }
    }
    res = res && doc->containsKey("ip");
    if(res){
      String _ip=(*doc)["ip"];
      Serial.printf("ip: %s\n", _ip.c_str());
      if(_ip==""||_ip=="0.0.0.0"){
        staticIp=false;
      }else{
        staticIp=true;
      }
      ip.fromString(_ip);
    }
    res = res && doc->containsKey("nm");
    if(res){
      String _nm=(*doc)["nm"];
      Serial.printf("nm: %s\n", _nm.c_str());
      nm.fromString(_nm);
    }
    res = res && doc->containsKey("gw");
    if(res){
      String _gw=(*doc)["gw"];
      Serial.printf("gw: %s\n", _gw.c_str());
      gw.fromString(_gw);
    }
    res= res && doc->containsKey("dns");
    if(res){
      for(int i=0;i<2;i++){
        String _dns = (*doc)["dns"][i];
        Serial.printf("dns[%i]: %s\n",i, _dns.c_str());
        dns[i].fromString(_dns);
      }
    }
  }
  Serial.println("End Config");
  free(buffer);
  free(doc);
  return res;
}

void saveConfigData(){
  if(!SPIFFS.begin()){
    Serial.println("Error opening spiffs");
    return;
  }
  File file = SPIFFS.open("/wifi.json", "w");
  if (!file) {
    Serial.println("Error opening file for writing");
    return;
  }

  int bytesWritten = file.print("{");
  if(isAP)bytesWritten += file.print("\"mode\":\"AP\",");
  else bytesWritten += file.print("\"mode\":\"STA\",");

  bytesWritten += file.print("\"SSID\":\""+ssid+"\",");
  bytesWritten += file.print("\"password\":\""+password+"\",");
  bytesWritten += file.print("\"hostname\":\""+hostname+"\",");
  if(!staticIp)bytesWritten += file.print("\"ip\":\"0.0.0.0\",");
  else bytesWritten += file.print("\"ip\":\""+ip.toString()+"\",");
  bytesWritten += file.print("\"nm\":\""+nm.toString()+"\",");
  bytesWritten += file.print("\"gw\":\""+gw.toString()+"\",");
  bytesWritten += file.print("\"dns\":[\""+dns[0].toString()+"\",\""+dns[1].toString()+"\"],");
  bytesWritten += file.print("\"webPass\":\""+webPass+"\"");
  bytesWritten += file.print("}");

  if (bytesWritten > 0) {
      Serial.println("File was written");
      Serial.println(bytesWritten);
  } else {
      Serial.println("File write failed");
  }	
  file.close();
}

void initWifi(){
  Serial.print("starting Wifi");
  LCD_setStatusMsg("Starting Wifi ...");
  delay(200);
  blinkTimes(3);
  bool bntCfg=isBtnCfg();
  clearBtnCfg();

  
  if(bntCfg || !loadConfigData()){
    initCP=true;
    isAP=true;
    ssid="LbEsp32-AP";
    password = "12345678";
    hostname = "lbesp32";
    staticIp=true;
    ip.fromString("192.168.4.1");
    nm.fromString("255.255.255.0");
    gw.fromString("192.168.4.1");
    dns[0].fromString("192.168.4.1");
    dns[1].fromString("8.8.8.8");
    webPass="lbesp32";
    saveConfigData();
  }

  if(staticIp){
    WiFi.config(ip,gw,nm,dns[0],dns[1]);
  }
  

  if(isAP){
    LCD_setStatusMsg("Mode AP ...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), password.c_str());
    if(!WiFi.softAPConfig(ip,gw,nm)){
      Serial.println("Error Setting IP");
    }
    WiFi.setHostname(hostname.c_str());
    LCD_blinkLED(2,3,2);
    Serial.print("AP Hostname: ");
    Serial.println(WiFi.getHostname());
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("AP GW address: ");
    Serial.println(WiFi.gatewayIP());
    WiFi.softAP(ssid.c_str(), password.c_str());
    LCD_setNetwork(ssid);
    if(initCP)LCD_setStatusMsg("Reset Config");
  }else{
    LCD_setStatusMsg("Connecting ...");
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    while(!WiFi.isConnected()){
      delay(100);
    }
    LCD_setNetwork(ssid);
  }
}

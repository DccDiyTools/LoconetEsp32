#include <Arduino.h>
#include "WebServer.h"
#include "lbWifi.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "ArduinoJson.h"
#include "lbLcd.h"

AsyncWebServer server(HTTP_PORT);
String webPass;

#define GET_AUTH     if(!request->authenticate(webUser,webPass.c_str())){ \
        return request->requestAuthentication(); \
    }

#define DEBUG_WS(text)                \
    Serial.print(F("[WebServer]: ")); \
    Serial.println(text);

static const char HEX_CHAR_ARRAY[17] = "0123456789ABCDEF";


uint32_t getChipID()
{
  uint64_t chipId64 = 0;

  for (int i = 0; i < 6; i++)
  {
    chipId64 |= ( ( (uint64_t) ESP.getEfuseMac() >> (40 - (i * 8)) ) & 0xff ) << (i * 8);
  }

  return (uint32_t) (chipId64 & 0xFFFFFF);
}

//////////////////////////////////////////

uint32_t getChipOUI()
{
  uint64_t chipId64 = 0;

  for (int i = 0; i < 6; i++)
  {
    chipId64 |= ( ( (uint64_t) ESP.getEfuseMac() >> (40 - (i * 8)) ) & 0xff ) << (i * 8);
  }

  return (uint32_t) (chipId64 >> 24);
}

String getESP32ChipID()
{
   return String(getChipID(),HEX);
}



void handleRoot(AsyncWebServerRequest *request)
{
    // AJS - maybe we should set a scan when we get to the root???
    // and only scan on demand? timer + on demand? plus a link to make it happen?

    //  shouldscan = true;
    //  scannow = 0;
    DEBUG_WS(F("Handle root"));

    // if (captivePortal(request))
    // {
    //     // if captive portal redirect instead of displaying the page
    //     return;
    // }
    GET_AUTH

    DEBUG_WS(F("Sending Captive Portal"));
    request->send(SPIFFS, "/web/index.html");
    DEBUG_WS(F("Sent..."));
}

void handleStatus(AsyncWebServerRequest *request)
{
    DEBUG_WS(F("Handle status"));
    GET_AUTH
    String json = "{";
    json+="\"name\":\"LB_"+getESP32ChipID()+"\",";
    json+="\"wifi\":{";
    json+="\"mode\":\"";
    json+=isAP?"AP":"Client";
    json+="\",";
    json+="\"SSID\":\""+ssid+"\",";
    json+="\"hostname\":\""+hostname+"\",";
    if(staticIp){
        json+="\"staticIp\":true,";
    }else{
        json+="\"staticIp\":false,";
    }
    if(staticIp){
        json+="\"ip\":\""+ip.toString()+"\",";
        json+="\"nm\":\""+nm.toString()+"\",";
        json+="\"gw\":\""+gw.toString()+"\",";
        json+="\"dns\":[\""+dns[0].toString()+"\",\""+dns[1].toString()+"\"]";
    }else{
        json+="\"ip\":\""+WiFi.localIP().toString()+"\",";
        json+="\"nm\":\""+WiFi.subnetMask().toString()+"\",";
        json+="\"gw\":\""+WiFi.gatewayIP().toString()+"\",";
        json+="\"dns\":[\""+WiFi.dnsIP(0).toString()+"\",\""+WiFi.dnsIP(1).toString()+"\"]";
    }
    json += "}}";

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
    json = String();
    DEBUG_WS(F("Sent status"));
}

void handleInfo(AsyncWebServerRequest *request){
    DEBUG_WS(F("Handle info"));
    GET_AUTH
    String json = "{";
    json+="\"chipId\":\""+getESP32ChipID()+"\",";
    json+="\"chipOID\":\""+String(getChipOUI(),HEX)+"\",";
    json+="\"chipModel\":\""+String(ESP.getChipModel())+" Rev"+String(ESP.getChipRevision(),DEC)+"\",";
    json+="\"flashId\":\""+String(ESP_getFlashChipId(),HEX)+"\",";
    json+="\"flashSize\":\""+String(ESP.getFlashChipSize(),DEC) +" bytes\",";
    json+="\"flashRealSize\":\""+String(ESP_getFlashChipRealSize(),DEC)+" Bytes\",";
    json+="\"apIP\":\""+WiFi.softAPIP().toString()+"\",";
    json+="\"apMAC\":\""+WiFi.softAPmacAddress()+"\",";
    json+="\"apSSID\":\""+WiFi.softAPSSID()+"\",";
    json+="\"staSSID\":\""+WiFi.SSID()+"\",";
    json+="\"staIP\":\""+WiFi.localIP().toString()+"\",";
    json+="\"staMAC\":\""+WiFi.macAddress()+"\"";
    json += "}";

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
    json = String();
    DEBUG_WS(F("Sent info"));
}

void handleSave(AsyncWebServerRequest *request){

    DEBUG_WS(F("Handle Save"));
    GET_AUTH


    String json = "{";
    // for (unsigned int i = 0; i < request->args(); i++)
    // {
    //     json+=(i!=0)?(", \""):" \"";
    //     json += request->argName(i) + "\": \"" + request->arg(i)+"\"";
    // }


    bool valid=true;
    int code=200;
    valid= valid && request->hasArg("mode");
    valid= valid && request->hasArg("ssid");
    valid= valid && request->hasArg("pwd");
    valid= valid && request->hasArg("ipMode");
    valid= valid && request->hasArg("ip");
    valid= valid && request->hasArg("gw");
    valid= valid && request->hasArg("sn");
    valid= valid && request->hasArg("dns1");
    valid= valid && request->hasArg("dns2");

    if(valid){
        if(!request->arg("mode").equals("")){
            isAP=request->arg("mode").equals("AP");
        }
        if(!request->arg("ssid").equals(""))ssid=request->arg("ssid");
        if(!request->arg("ssid").equals(""))password=request->arg("pwd");
        if(!request->arg("ipMode").equals("")){
            staticIp=request->arg("ipMode").equals("static");
        }

        if(!request->arg("ip").equals("")){
            if(request->arg("ip").equals("0.0.0.0")){
                staticIp=false;
            }else{
                valid= valid && ip.fromString(request->arg("ip"));
            }
        }if(!request->arg("gw").equals(""))valid= valid && gw.fromString(request->arg("gw"));
        if(!request->arg("sn").equals(""))valid= valid && nm.fromString(request->arg("sn"));
        if(!request->arg("dns1").equals(""))valid= valid && dns[0].fromString(request->arg("dns1"));
        if(!request->arg("dns2").equals(""))valid= valid && dns[1].fromString(request->arg("dns2"));
    }
    if(!valid){
        code=400;
        json+="\"error\":\"Invalid Parameters\"";
        loadConfigData();
    }else{
        saveConfigData();
    }

    json += "}";
    DEBUG_WS(json);

    AsyncWebServerResponse *response = request->beginResponse(code, "application/json", json);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
    LCD_blinkLED(2);
    digitalWrite(LED_G,0);
    digitalWrite(LED_B,0);
    json = String();
    DEBUG_WS(F("Sent Save"));
    if(valid){
        initWifi();
    }
}



void handleWifiScan(AsyncWebServerRequest *request)
{
    DEBUG_WS(F("Handle Scan"));
    GET_AUTH
  String json = "[";
  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks(true);
  } else if(n){
    for (int i = 0; i < n; ++i){
      if(i) json += ",";
      json += "{";
      json += "\"rssi\":"+String(WiFi.RSSI(i));
      json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
      json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
      json += ",\"channel\":"+String(WiFi.channel(i));
      json += ",\"secure\":"+String(WiFi.encryptionType(i));
      json += "}";
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  json += "]";

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
    LCD_blinkLED(5,1,1);
    json = String();
    DEBUG_WS(F("Sent Scan"));
}




// handle the reset page
void handleReset(AsyncWebServerRequest *request)
{
    DEBUG_WS(F("Reset"));
    GET_AUTH
    String json="\"Module will restart in ";
    json+=RESTART_TIME_OUT;
    json+="ms\"";
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);

    DEBUG_WS(F("Sent reset page"));
    delay(RESTART_TIME_OUT);
    ESP.restart();
}

void handleNotFound(AsyncWebServerRequest *request)
{
    DEBUG_WS(F("Handle not found"));
    // if (captivePortal(request))
    // {
    //     // if captive portal redirect instead of displaying the error page
    //     return;
    // }

    String message = "File Not Found\n\n";
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += (request->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";

    for (unsigned int i = 0; i < request->args(); i++)
    {
        message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
    }
    AsyncWebServerResponse *response = request->beginResponse(404, "text/plain", message);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
}

void initWebServer()
{
    server.onNotFound(handleNotFound);
    server.on("/", HTTP_GET, handleRoot);
    server.on("/reset", HTTP_POST, handleReset);
    server.on("/scan", HTTP_GET, handleWifiScan);
    server.on("/status",HTTP_GET,handleStatus);
    server.on("/info",HTTP_GET,handleInfo);
    server.on("/wifisave",HTTP_POST,handleSave);
    server.begin();
}



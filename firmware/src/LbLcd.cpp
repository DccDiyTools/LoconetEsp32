#include "lbLcd.h"
#include "lbWifi.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "WebServer.h"
#include <WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define MS_WAIT 100
uint32_t lcdTimer=0;


typedef enum lcdMode{
    LCD_PRE,
    LCD_IP,
    LCD_HOST_NAME,
    LCD_MDNS_NAME,
    LCD_PW,
    LCD_PW_SHOW,
    LCD_WEB_PW,
    LCD_WEB_PW_SHOW,
    LCD_NONE
} t_lcdMode;

typedef enum LedBlinkMode{
    BLINK_STOP,
    BLINK_ON,
    BLINK_OFF,
} t_ledBlinkMode;

uint8_t blinkOff=2; //*100ms
uint8_t blinkOn=2;  //*100ms
uint8_t blinkWait=0;
uint8_t blinkCnt=0;
t_ledBlinkMode blinkMode=BLINK_STOP;
t_lcdMode lcdMode=LCD_PRE;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

volatile bool btnPush=false;
volatile bool btnCfg=false;

void IRAM_ATTR btnIsr() {
  btnPush=true;
}
void IRAM_ATTR cfgIsr() {
  btnCfg=true;
}

bool isBtnCfg(){
    return btnCfg;
}
void clearBtnCfg(){
    btnCfg=false;
}

void initLcd()
{
    Wire.begin(LCD_SDA, LCD_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        return;
    }
    //delay(2000);
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(15, 10);
    //display.drawFastHLine(128-15,10,15,WHITE); //Debug
    // Display static text
    display.println("LocoBuffer ESP32");
    display.display();
    attachInterrupt(PIN_CFG, cfgIsr, RISING);
    attachInterrupt(BTN, btnIsr, RISING);
}

void LCD_setStatusMsg(String status){
    display.fillRect(0,55,SCREEN_WIDTH, 9,BLACK);
    display.setTextColor(WHITE);
    display.setCursor(1,55);
    display.println(status.c_str());
    //display.drawRect(0,0,128,65,WHITE); //Debug
    display.display();
}

void LCD_setNetwork(String network){
    display.clearDisplay();
    if (isAP){
        display.setCursor(0, 1);
        display.print("AP: ");
        display.print(network);
    }else{
        display.setCursor(0, 1);
        display.print(" C: ");
        display.print(network);
    }
    display.display();
}

void LCD_blinkLED(uint8_t blinks, uint8_t onDelay, uint8_t offDelay){
    blinkCnt=blinks;
    blinkOn=onDelay;
    blinkOff=offDelay;
}

void updateLed(){
    if(blinkMode==BLINK_STOP){
        if(blinkCnt>0){
            blinkMode=BLINK_OFF;
            blinkWait=blinkOff;
            blinkCnt++;
        }
    }else if(blinkMode==BLINK_ON){
        blinkWait--;
        if(blinkWait==0){
            blinkMode=BLINK_OFF;
            blinkWait=blinkOff;
        }
    }else if(blinkMode==BLINK_OFF){
        blinkWait--;
        if(blinkWait==0){
            blinkMode=BLINK_ON;
            blinkWait=blinkOn;
            blinkCnt--;
            if(blinkCnt==0){
                blinkWait=0;
                blinkMode=BLINK_STOP;
            }
        }
    }
}

void updateBeat(){
    if(blinkMode==BLINK_STOP){
        blinkWait--;
        digitalWrite(isAP?LED_B:LED_G,(blinkWait>0)?1:0);
        if(blinkWait==0)blinkWait=100;
    }
}

void updateMode() {
    switch (lcdMode)
    {
    case LCD_PRE:
        lcdMode=LCD_IP;
        break;
    case LCD_IP:
        if(btnPush)lcdMode=LCD_HOST_NAME;
        break;
    case LCD_HOST_NAME:
        if(btnPush)lcdMode=LCD_MDNS_NAME;
        break;
    case LCD_MDNS_NAME:
        if(btnPush)lcdMode=(isAP)?LCD_PW:LCD_WEB_PW;
        break;
    case LCD_PW:
        if(btnPush)lcdMode=LCD_WEB_PW;
        if(btnCfg)lcdMode=LCD_PW_SHOW;
        break;
    case LCD_PW_SHOW:
        if(btnPush)lcdMode=LCD_WEB_PW;
        if(btnCfg)lcdMode=LCD_PW;
        break;
    case LCD_WEB_PW:
        if(btnPush)lcdMode=LCD_IP;
        if(btnCfg)lcdMode=LCD_WEB_PW_SHOW;
        break;
    case LCD_WEB_PW_SHOW:
        if(btnPush)lcdMode=LCD_IP;
        if(btnCfg)lcdMode=LCD_WEB_PW;
        break;
    default:
        break;
    }
}

void printMode(){
    display.fillRect(0,12,SCREEN_WIDTH, 9,BLACK);
    display.setCursor(0,12);
    switch (lcdMode)
    {
    case LCD_PRE:
        break;
    case LCD_IP:
        display.printf("IP: %s",WiFi.localIP().toString().c_str());
        break;
    case LCD_HOST_NAME:
        display.printf("Host: %s",WiFi.getHostname());
        break;
    case LCD_MDNS_NAME:
        display.printf("mDns: %s.local",WiFi.getHostname());
        break;
    case LCD_PW:
        display.printf("WiFi PW");
        break;
    case LCD_PW_SHOW:
        display.printf(password.c_str());
        break;
    case LCD_WEB_PW:
        display.printf("Web PWD");
        break;
    case LCD_WEB_PW_SHOW:
        display.printf("admin/%s",webPass.c_str());
        break;
    default:
        break;
    }
}

void updateLoop(){
    updateLed();
    digitalWrite(isAP?LED_B:LED_G,(blinkMode==BLINK_ON)?1:0);
    updateBeat();

    updateMode();
    if(btnPush || btnCfg){
        btnPush=false;
        btnCfg=false;
    }
    printMode();
    display.display();
}


void LCD_update_loop(){
    if(lcdTimer==0){
        lcdTimer=millis()+MS_WAIT;
    }
    if (millis()>lcdTimer){
        updateLoop();
        lcdTimer+=MS_WAIT;
    }
}
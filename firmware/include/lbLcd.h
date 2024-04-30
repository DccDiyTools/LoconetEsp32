#ifndef LB_LCD_H
#define LB_LCD_H
#include <Arduino.h>

#define LCD_SDA 26
#define LCD_SCL 27

#define LED_R 16
#define LED_G 17
#define LED_B 18

#define BTN 33
#define PIN_CFG 32

void initLcd();
void LCD_setStatusMsg(String status);
void LCD_setNetwork(String network);
void LCD_update_loop();
void LCD_blinkLED(uint8_t blinks, uint8_t onDelay=5, uint8_t offDelay=5);

bool isBtnCfg();
void clearBtnCfg();
#endif
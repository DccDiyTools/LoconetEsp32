#ifndef LOCONET_H
#define LOCONET_H

#include <IoTT_lbServer.h>

/////////USER CONFIGURATION//////////////////////////////////////////
#define pinRx    22  //pin used to receive LocoNet signals
#define pinTx    23  //pin used to transmit LocoNet signals
#define pinNop   30  //unused pin to set for the hardware Serial port as transmit pin
#define InverseLogic true
//#define showPerformance
/////////END OF USER CONFIGURATION//////////////////////////////////////////
extern IoTT_LBServer * lbServer;
extern LocoNetESPSerial lnSerial;
#endif
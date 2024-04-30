#include <Arduino.h>
#include <IoTT_LocoNetHBESP32.h> 
#include <IoTT_lbServer.h>
#include "loconet.h"

void callbackLocoNetMessage(lnReceiveBuffer * newData)
{
   Serial.printf("LN Msg Error Flags %2X ReqID %i with %i bytes requested %i response time %i echo time %i: ", newData->errorFlags, newData->requestID, newData->lnMsgSize, newData->reqRecTime, newData->reqRespTime, newData->echoTime); 
   for (int i=0; i<newData->lnMsgSize; i++)
   {
     Serial.print(newData->lnData[i],16);
     Serial.print(" ");
   }
   Serial.println();
   lbServer->lnWriteMsg(newData);
}

void callbackLocoNetMessageUpstream(lnReceiveBuffer * newData)
{
   Serial.printf("LNU Msg Error Flags %2X ReqID %i with %i bytes requested %i response time %i echo time %i: ", newData->errorFlags, newData->requestID, newData->lnMsgSize, newData->reqRecTime, newData->reqRespTime, newData->echoTime); 
   for (int i=0; i<newData->lnMsgSize; i++)
   {
     Serial.print(newData->lnData[i],16);
     Serial.print(" ");
   }
   Serial.println();
}
 

uint16_t sendMsg(lnTransmitMsg txData)
{
//  Serial.printf("verifySyntax %i\n", useInterface.devId);
//  if (!verifySyntax(&txData.lnData[0]))
//  {
//    Serial.printf("ERROR: Call sendMsg to %i: %i, %2X, %2X, %2X, %2X \n", useInterface.devId, txData.lnMsgSize, txData.lnData[0], txData.lnData[1], txData.lnData[2], txData.lnData[3]);
//    Serial.printf("Outgoing Callback to %i ID %2X\n", useInterface.devId, txData.requestID);
//    return 0;
//  }
lnTransmitMsg * newData=&txData;
Serial.printf("LB Msg :ReqID %i with %i bytes requested %i RequestTime ", 
newData->requestID, newData->lnMsgSize, newData->reqRecTime); 
   for (int i=0; i<newData->lnMsgSize; i++)
   {
     Serial.print(newData->lnData[i],16);
     Serial.print(" ");
   }
   Serial.println();

  return lnSerial.lnWriteMsg(newData);
  //return 0;
}
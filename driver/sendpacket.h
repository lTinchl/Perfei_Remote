#ifndef _sendpacket_h_
#define _sendpacket_h_

#include "stm32f10x.h"
#include "nrf24l01.h"

typedef struct
{
    uint16_t thr;
    uint8_t pit;
    uint8_t rol;
    uint8_t yaw;
    uint8_t key;
    
}RemoteData;


extern RemoteData tx;
extern uint8_t pairModeEnabled;
extern u8 rxPacketStatus; 
extern u8 rxPacket[TX_PLOAD_WIDTH];
extern float remoteVoltage;


void NrfTxPacket(void);
void NrfRxPacket(void);
void OledDisplayPairStatus(void);
 
#endif


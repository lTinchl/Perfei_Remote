#include "sendpacket.h"
#include "timing_trigger.h"
#include "usart1.h"
#include "adc.h"
#include "nrf24l01.h"
#include "key.h"
#include "led.h"
#include "pair_freq.h"
#include "u8g2.h"
#include "compile.h"
#include <stm32f10x_dma.h>

RemoteData tx = {0};
extern Pair pair;
extern float remoteVoltage;

extern u8g2_t u8g2;

u8 rxPacket[TX_PLOAD_WIDTH] = {0};
u8 rxPacketStatus;
u8 pairModeEnabled = 1;  // 默认不开启对频显示
volatile uint8_t adc_dma_done = 0;


void analyze_packet(uint16_t *adcData)
{
#if FOUR_AXIS_UAV
		//油门采集的数据量化为 0 ~ 1000
		//舵面采集的数据量化为 0 ~ 100
    tx.thr = (uint16_t)(((float)(adcData[1]))*0.24420f);
    tx.yaw = (uint8_t)(((float)(adcData[0]))*0.02442f);
    tx.pit = (uint8_t)(((float)(adcData[3]))*0.02442f);
    tx.rol = (uint8_t)(((float)(adcData[2]))*0.02442f);
#elif FIXED_WING_AIRCRAFT
    tx.thr = (uint16_t)(((float)(adcData[1]))*0.24420f);
    tx.yaw = (uint8_t)(((float)(adcData[0]))*0.02442f);
    tx.pit = (uint8_t)(((float)(adcData[3]))*0.02442f);
    tx.rol = (uint8_t)(((float)(adcData[2]))*0.02442f);
#elif BRUSHLESS_FOUR_AXIS_UAV
		//油门采集的数据量化为 4000 ~ 8000
		//舵面采集的数据量化为 0 ~ 400
    tx.thr = (uint16_t)(((float)(adcData[1]))*0.9765625f + 4000);//4000~8000
    tx.yaw = (uint16_t)(((float)(adcData[0]))*0.09765625f);
    tx.pit = (uint16_t)(((float)(adcData[3]))*0.09765625f);
    tx.rol = (uint16_t)(((float)(adcData[2]))*0.09765625f);
#endif
}

void data_exchange(uint8_t *dateBuff)
{
	//发送新的地址和频点给飞机
	if(pair.step == STEP1){
			dateBuff[0] = 0xA8;					//帧头
			dateBuff[1] = pair.addr[0];		  //5个字节地址    
			dateBuff[2] = pair.addr[1];	    //    
			dateBuff[3] = pair.addr[2];			//	
			dateBuff[4] = pair.addr[3];			//	
			dateBuff[5] = pair.addr[4];			//	
			
			dateBuff[6] = pair.freq_channel;		//通信频点		


			dateBuff[TX_PLOAD_WIDTH-1] = 0x8B;	//帧尾      
	}
	//正常数据通信
	else if(pair.step == DONE){
		#if FOUR_AXIS_UAV
			dateBuff[0] = 0xA8;		//帧头
			dateBuff[1] = *((u8*)&tx.thr);		//油门低八位
			dateBuff[2] = *(((u8*)&tx.thr)+1);		//油门高八位
			dateBuff[3] = tx.pit;		//俯仰舵向
			dateBuff[4] = tx.rol;		//横滚舵向
			dateBuff[5] = tx.yaw;		//偏航舵向
			dateBuff[6] = key.l;		//左功能按键
			dateBuff[7] = key.r;		//右功能按键
			dateBuff[8] = 0;		//未使用，可自行添加
			dateBuff[9] = 0;		//未使用，可自行添加
			dateBuff[TX_PLOAD_WIDTH-1] = 0x8B;		//帧尾 
		#elif FIXED_WING_AIRCRAFT
			dateBuff[0] = 0xA8;		//帧头
			dateBuff[1] = *((u8*)&tx.thr);		//油门低八位
			dateBuff[2] = *(((u8*)&tx.thr)+1);		//油门高八位
			dateBuff[3] = tx.pit;		//俯仰舵向
			dateBuff[4] = tx.rol;		//横滚舵向
			dateBuff[5] = tx.yaw;		//偏航舵向
			dateBuff[6] = key.l1;		//左1功能按键
			dateBuff[7] = key.r1;		//右1功能按键
			dateBuff[8] = key.l2;		//左2功能按键
			dateBuff[9] = key.r2;		//右2功能按键
			dateBuff[TX_PLOAD_WIDTH-1] = 0x8B;		//帧尾 
		#elif BRUSHLESS_FOUR_AXIS_UAV
			dateBuff[0] = 0xA8;		//帧头
			dateBuff[1] = *((u8*)&tx.thr);		//油门低八位
			dateBuff[2] = *(((u8*)&tx.thr)+1);		//油门高八位
			dateBuff[3] = *((u8*)&tx.pit);		//俯仰舵向
			dateBuff[4] = *(((u8*)&tx.pit)+1);		//俯仰舵向
			dateBuff[5] = *((u8*)&tx.rol);		//横滚舵向
			dateBuff[6] = *((u8*)&tx.rol+1);		//横滚舵向
			dateBuff[7] = *((u8*)&tx.yaw);		//偏航舵向
			dateBuff[8] = *((u8*)&tx.yaw+1);		//偏航舵向
			dateBuff[9] = key.l;		//左功能按键
			dateBuff[10] = key.r;		//右功能按键
			dateBuff[11] = 0;		//未使用，可自行添加
			dateBuff[12] = 0;		//未使用，可自行添加
			dateBuff[TX_PLOAD_WIDTH-1] = 0x8B;		//帧尾 
		#endif     
	}
}

uint8_t tx_dat[TX_PLOAD_WIDTH] = {0};


//DMA通道1中断触发，如果触发，则表明DMA已经将ADC数据搬运到内存数组中，则可以进行发送数据了    
void DMA1_Channel1_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC1) == SET){	
        adc_dma_done = 1; // 设置标志，主循环做分析和发包
        DMA_ClearITPendingBit(DMA1_IT_TC1);		
    }
}

//发送数据包
void NrfTxPacket(void)
{
    if(adc_dma_done) {
        adc_dma_done = 0;
        analyze_packet(ADC_value);		
        data_exchange(tx_dat);
        uint8_t ret = NRF24L01_TxPacket(tx_dat);
        if(ret != TX_OK){
            LedBlink(YELLOW);
        }
    }
}


//接收数据包
void NrfRxPacket(void)
{
	NRF24L01_Read_Buf(RD_RX_PLOAD,rxPacket,11);
	if(rxPacket[0] == 0XAA && rxPacket[TX_PLOAD_WIDTH - 1] == 0XAC){//检查包头包尾
		rxPacketStatus = 1;  
	} else {
		rxPacketStatus = 0;
	}
	
}





#include "stm32f10x.h"
#include "systick.h"
#include "spi.h"
#include "nrf24l01.h"
#include "usart1.h"
#include "timing_trigger.h"
#include "nvic.h"
#include "key.h"
#include "adc.h"
#include "pair_freq.h"
#include "sendpacket.h"
#include "RCC.h"
#include "OLED_UI.h"
#include "OLED_UI_MenuData.h"


int main(void)
{
    /* 模块初始化 */
    SystemInit();
    systick_init();
	
    Usart1Init(115200);
    get_chip_id();

    SPI1_Init();
    NRF24L01_Init();
    NRF24L01_Check();
    NRF24L01_TX_Mode();

    KeyInit();
    ADC_Config();
    timing_trigger_init();
    Timer_Init();
    NVIC_config();

    delay_ms(100);
    RCC_HSE_Configuration();
	
    OLED_UI_Init(&MainMenuPage);

    while (1)
    {
        NrfTxPacket();  //发包
        OLED_UI_MainLoop();  // 主菜单
        WaitPairing();  //对频函数
        // OledDisplayPairStatus();
    }
}

//中断函数
void TIM3_IRQHandler(void)
{
	  if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        OLED_UI_InterruptHandler();   // UI 每20ms刷新
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}


#include "stm32f10x.h"
#include "systick.h"
#include "led.h"
#include "spi.h"
#include "nrf24l01.h"
#include "usart1.h"
#include "timing_trigger.h"
#include "nvic.h"
#include "adc.h"
#include "key.h"
#include "pair_freq.h"
#include "sendpacket.h"
#include "IIC_OLED.h"
#include "u8g2.h"
#include "RCC.h"
#include "menu.h"
#include "ui.h"
#include "OLED_UI.h"
#include "OLED_UI_MenuData.h"


int main(void)
{
    /* 模块初始化 */
    SystemInit();
    systick_init();

    //LedInit();
    Usart1Init(115200);
    get_chip_id();

    SPI1_Init();
    NRF24L01_Init();
    while (NRF24L01_Check())
    {
        LedBlink(RED);
    }
    NRF24L01_TX_Mode();

    KeyInit();
    ADC_Config();
    timing_trigger_init();
    Timer_Init();
    NVIC_config();

    delay_ms(100);
    RCC_HSE_Configuration();
	
    OLED_UI_Init(&MainMenuPage);
    // u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8g2_gpio_and_delay_stm32);
    // u8g2_InitDisplay(&u8g2);
    // u8g2_SetPowerSave(&u8g2, 0);

    while (1)
    {
        NrfTxPacket();  //发包
        OLED_UI_MainLoop(); 
        // if(menu_state == MENU_SET_ENTER)
        // {second_menu();}
        // else
        // {main_menu();}
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


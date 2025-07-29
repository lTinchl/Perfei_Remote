#include "stm32f10x.h"
#include "systick.h"
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
#include "ui.h"

u8g2_t u8g2;

void main_menu(void)
{
    u8g2_ClearBuffer(&u8g2);   // 清屏
    DisplayInfo();             // 电压/锁定状态
    Remotecontroldata();       // 丝滑进度条
    menu_fps(&u8g2);           // 右下角FPS显示
		ui_icon();                 // 始终渲染图标文字
    u8g2_SendBuffer(&u8g2);    // 推送到屏幕
}


void no2_menu(void)
{
   u8g2_ClearBuffer(&u8g2);    //清屏
   
   u8g2_SendBuffer(&u8g2);    // 推送到屏幕
}


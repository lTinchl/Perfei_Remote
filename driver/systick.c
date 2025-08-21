#include "stm32f10x.h"
#include "systick.h"
#include "OLED_driver.h"

volatile uint32_t tick_count = 0;  // 需要 volatile
volatile uint32_t millis_count = 0;


void systick_init(void)
{
    SysTick->LOAD  = SystemCoreClock / 1000 - 1UL;  // 1ms
    SysTick->VAL   = 0UL;
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk |
                     SysTick_CTRL_ENABLE_Msk;      // 启用计数 + 中断

    __enable_irq();  // 开启全局中断
}


// us级延时
void delay_us(uint32_t us)
{
    while(us--)
    {
        // 这个 NOP 空转约等于 1us（取决于主频，可微调）
        for (int i = 0; i < 8; i++)
            __NOP();
    }
}


// ms级延时
void delay_ms(uint32_t time)
{
    while (time--) delay_us(1000);
}

// SysTick 中断函数
void SysTick_Handler(void)
{
    millis_count++;
}


uint32_t systick_get_ms(void)
{
    return millis_count;
}



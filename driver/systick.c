#include "stm32f10x.h"
#include "systick.h"

/* 系统滴筒定时器实现可用于延时 + u8g2 使用 */

#include "u8g2.h"
#include "IIC_OLED.h"

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


// u8g2 在 STM32 下用于 software I2C 的 GPIO 和延时选项函数
uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_DELAY_MILLI:
            delay_ms(arg_int);
            break;

        case U8X8_MSG_DELAY_10MICRO:
            delay_us(10);
            break;

        case U8X8_MSG_DELAY_100NANO:
            __NOP();  // 缓行一个命令周期
            break;

        case U8X8_MSG_GPIO_I2C_CLOCK:
            if (arg_int)
                IIC_OLED_SCL_HIGH();
            else
                IIC_OLED_SCL_LOW();
            break;

        case U8X8_MSG_GPIO_I2C_DATA:
            if (arg_int)
                IIC_OLED_SDA_HIGH();
            else
                IIC_OLED_SDA_LOW();
            break;

        default:
            return 0; // 未实现的信息
    }
    return 1;
}

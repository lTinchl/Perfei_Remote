#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"
#include "u8g2.h"

// 全局函数声明
void systick_init(void);
void delay_us(uint32_t time);
void delay_ms(uint32_t time);
uint32_t systick_get_ms(void);

// u8g2 软件 I2C GPIO/延时接口函数声明
uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8,
                                  U8X8_UNUSED uint8_t msg,
                                  U8X8_UNUSED uint8_t arg_int,
                                  U8X8_UNUSED void *arg_ptr);

#endif

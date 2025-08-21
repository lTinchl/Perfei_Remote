#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

// 全局函数声明
void systick_init(void);
void delay_us(uint32_t time);
void delay_ms(uint32_t time);
uint32_t systick_get_ms(void);


#endif

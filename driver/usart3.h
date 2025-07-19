#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

void usart3_init(u32 bound);
void USART3_DMA_SEND_DATA(u32 SendBuff, u16 len);

#endif

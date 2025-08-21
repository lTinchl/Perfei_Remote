#ifndef __OLED_DRIVER_H
#define __OLED_DRIVER_H



#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

//	oled��ʼ������
void OLED_Init(void);
// ������ɫģʽ
void OLED_SetColorMode(bool colormode);
//	oledȫ��ˢ�º���
void OLED_Update(void);
//	oled�ֲ�ˢ�º���
void OLED_UpdateArea(uint8_t X, uint8_t Y, uint8_t Width, uint8_t Height);


// OLED �������Ⱥ���
void OLED_Brightness(int16_t Brightness);







#endif








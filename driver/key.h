#ifndef _key_h_
#define _key_h_

#include "stm32f10x.h"

//菜单层级
#define MAX_MENU_LEVEL 3

typedef struct
{
    uint8_t value;     						// 当前按键值（1~4表示哪个键，0表示无键按下）
    uint8_t l1, r1, l2, r2; 				// 分别记录四个按键的事件或编码
    uint16_t l1_num, r1_num, l2_num, r2_num; // 四个按键按下计数
}_KEY;

typedef struct {
    uint8_t level;                 // 当前菜单层级：0-主菜单，1-二级菜单，2-三级菜单
    uint8_t index[MAX_MENU_LEVEL]; // 每层当前选中的菜单项索引
} MenuContext;

extern MenuContext menu_ctx;


// 时钟宏定义
#define key_l1_rcc    RCC_APB2Periph_GPIOC
#define key_r1_rcc    RCC_APB2Periph_GPIOA
#define key_l2_rcc    RCC_APB2Periph_GPIOB
#define key_r2_rcc    RCC_APB2Periph_GPIOB

// 端口宏定义
#define key_l1_port   GPIOC
#define key_r1_port   GPIOA
#define key_l2_port   GPIOB
#define key_r2_port   GPIOB

// 引脚宏定义
#define key_l1_pin    GPIO_Pin_13  // 左1
#define key_r1_pin    GPIO_Pin_8   // 右1
#define key_l2_pin    GPIO_Pin_12  // 左2
#define key_r2_pin    GPIO_Pin_15  // 右2

// 电平读取
#define key_l1   GPIO_ReadInputDataBit(key_l1_port, key_l1_pin)
#define key_r1   GPIO_ReadInputDataBit(key_r1_port, key_r1_pin)
#define key_l2   GPIO_ReadInputDataBit(key_l2_port, key_l2_pin)
#define key_r2   GPIO_ReadInputDataBit(key_r2_port, key_r2_pin)

extern _KEY key;

void KeyInit(void);
uint8_t KeyScan(uint8_t mode);
void key_info(void);
void menu_process(uint8_t key_value);

#endif


#include "key.h"
#include "systick.h"
#include "ui.h"

MenuState menu_state = MENU_IDLE;

_KEY key = {0};

void KeyInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能时钟
    RCC_APB2PeriphClockCmd(key_l1_rcc, ENABLE);
    RCC_APB2PeriphClockCmd(key_r1_rcc, ENABLE);
    RCC_APB2PeriphClockCmd(key_l2_rcc, ENABLE);
    RCC_APB2PeriphClockCmd(key_r2_rcc, ENABLE);

    // 左1 PC13
    GPIO_InitStructure.GPIO_Pin = key_l1_pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(key_l1_port, &GPIO_InitStructure);

    // 右1 PA8
    GPIO_InitStructure.GPIO_Pin = key_r1_pin;
    GPIO_Init(key_r1_port, &GPIO_InitStructure);

    // 左2 PB12
    GPIO_InitStructure.GPIO_Pin = key_l2_pin;
    GPIO_Init(key_l2_port, &GPIO_InitStructure);

    // 右2 PB15
    GPIO_InitStructure.GPIO_Pin = key_r2_pin;
    GPIO_Init(key_r2_port, &GPIO_InitStructure);
}


//按键扫描
//0：不支持连续按
//1：支持连续按
uint8_t KeyScan(uint8_t mode)
{
    static uint8_t key_up = 1;
    if(mode) key_up = 1;

    if(key_up && (key_l1==0 || key_r1==0 || key_l2==0 || key_r2==0)) {
        delay_ms(5);
        key_up = 0;
        if(key_l1 == 0)   return 1; // 左1
        if(key_r1 == 0)   return 2; // 右1
        if(key_l2 == 0)   return 3; // 左2
        if(key_r2 == 0)   return 4; // 右2
    } else if(key_l1==1 && key_r1==1 && key_l2==1 && key_r2==1) {
        key_up = 1;
    }
    return 0;
}

void key_info(void)
{
    static uint8_t last_key_value = 0;

    key.value = KeyScan(0); // 单击有效
    key.l1 = (key.value == 1) ? 0xE1 : 0;
    key.r1 = (key.value == 2) ? 0xC8 : 0;
    key.l2 = (key.value == 3) ? 0xA1 : 0;
    key.r2 = (key.value == 4) ? 0xB8 : 0;

    if(!key.value) {
        key.l1 = 0;
        key.r1 = 0;
        key.l2 = 0;
        key.r2 = 0;
    }

    // 菜单状态机
    if(key.value && key.value != last_key_value)
    {
        switch(key.value)
        {
            case 1: // 左1：选中/进入
                if(menu_state == MENU_IDLE)
                    menu_state = MENU_SET_HIGHLIGHT;
                else if(menu_state == MENU_SET_HIGHLIGHT)
                    menu_state = MENU_SET_ENTER;
                else if(menu_state == MENU_SET_ENTER) {
                    // 二级菜单中确认操作可写在这里
                }
                break;
            case 2: // 右1：向上切换菜单项
                if(menu_state == MENU_SET_HIGHLIGHT) {
                    if(menu_index == 0) menu_index = menu_count - 1;
                    else menu_index--;
                }
                break;
            case 3: // 左2：返回/取消
                if(menu_state == MENU_SET_HIGHLIGHT)
                    menu_state = MENU_IDLE;
                else if(menu_state == MENU_SET_ENTER)
                    menu_state = MENU_SET_HIGHLIGHT;
                break;
            case 4: // 右2：向下切换菜单项
                if(menu_state == MENU_SET_HIGHLIGHT) {
                    menu_index++;
                    if(menu_index >= menu_count) menu_index = 0;
                }
                break;
        }
    }
    last_key_value = key.value;
}


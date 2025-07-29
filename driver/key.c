#include "key.h"
#include "systick.h"
#include "ui.h"

MenuState menu_state = MENU_IDLE;
MenuContext menu_ctx  = {0, {0}};


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

void menu_process(uint8_t key_value) {
    switch(key_value) {
        case 1: // 确认/进入
            if(menu_state == MENU_IDLE) {
                menu_state = MENU_SET_HIGHLIGHT;
                menu_ctx.level = 0;  // 进入主菜单
                menu_ctx.index[0] = 0;
            } else if(menu_state == MENU_SET_HIGHLIGHT) {
                menu_state = MENU_SET_ENTER;
            } else if(menu_state == MENU_SET_ENTER) {
                if(menu_ctx.level < MAX_MENU_LEVEL - 1) {
                    // 进入下一级菜单
                    menu_ctx.level++;
                    menu_ctx.index[menu_ctx.level] = 0;  // 下一级默认选中第一项
                    menu_state = MENU_SET_HIGHLIGHT;
                } else {
                    // 到最深层，执行确认操作
                    // todo: 根据菜单索引执行对应功能
                }
            }
            break;
        case 2: // 上
            if(menu_state == MENU_SET_HIGHLIGHT) {
                if(menu_ctx.index[menu_ctx.level] == 0)
                    menu_ctx.index[menu_ctx.level] = menu_count - 1;
                else
                    menu_ctx.index[menu_ctx.level]--;
            }
            break;
        case 3: // 返回/取消
            if(menu_state == MENU_SET_HIGHLIGHT) {
                if(menu_ctx.level == 0) {
                    menu_state = MENU_IDLE;  // 退出菜单
                } else {
                    // 返回上一层菜单
                    menu_ctx.index[menu_ctx.level] = 0;  // 清空当前层选择
                    menu_ctx.level--;
                    menu_state = MENU_SET_HIGHLIGHT;
                }
            } else if(menu_state == MENU_SET_ENTER) {
                menu_state = MENU_SET_HIGHLIGHT;
            }
            break;
        case 4: // 下
            if(menu_state == MENU_SET_HIGHLIGHT) {
                menu_ctx.index[menu_ctx.level]++;
                if(menu_ctx.index[menu_ctx.level] >= menu_count)
                    menu_ctx.index[menu_ctx.level] = 0;
            }
            break;
    }
}

void key_info(void) {
    uint8_t key_value = KeyScan(0);
    static uint8_t last_key_value = 0;

    if(key_value && key_value != last_key_value) {
        menu_process(key_value);
    }
    last_key_value = key_value;
}




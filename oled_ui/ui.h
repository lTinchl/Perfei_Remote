#ifndef _UI_H_
#define _UI_H_

#include <stdint.h>
#include "stm32f10x.h"

typedef enum {
    MENU_IDLE = 0,          // 主界面
    MENU_SET_HIGHLIGHT,     // 菜单高亮状态（选中当前层级菜单项）
    MENU_SET_ENTER,         // 进入当前层级菜单（准备进入下一层）
} MenuState;

extern MenuState menu_state;

extern uint8_t menu_index;
extern const uint8_t menu_count;

void menu_fps(u8g2_t *u8g2);
void DisplayInfo(void);
void main_muenu_ui(void);
void second_menu_ui(void);
void Remotecontroldata(void);

#endif  // _UI_H_

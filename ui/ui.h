#ifndef _UI_H_
#define _UI_H_

#include <stdint.h>
#include "stm32f10x.h"

typedef enum {
    MENU_IDLE = 0,          // 初始状态
    MENU_SET_HIGHLIGHT,     // 菜单高亮
    MENU_SET_ENTER          // 进入二级菜单
} MenuState;

extern MenuState menu_state;

extern uint8_t menu_index;
extern const uint8_t menu_count;

void menu_fps(u8g2_t *u8g2);
void DisplayInfo(void);
void ui_icon(void);
void Remotecontroldata(void);

#endif  // _UI_H_

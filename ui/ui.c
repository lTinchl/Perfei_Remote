#include "stm32f10x.h"
#include "systick.h"
#include "spi.h"
#include "nrf24l01.h"
#include "usart1.h"
#include "timing_trigger.h"
#include "nvic.h"
#include "adc.h"
#include "key.h"
#include "pair_freq.h"
#include "sendpacket.h"
#include "IIC_OLED.h"
#include "u8g2.h"
#include "ui.h"

extern u8g2_t u8g2;
extern Pair pair;
extern uint16_t ADC_value[5];

// 图标状态变量
static u8 lastSignalStatus = 0xFF; // 初始化为非法值，确保第一次刷新

uint8_t menu_index = 0;
const uint8_t menu_count = 2;

// 滤波缓冲变量（取代直接使用 tx 结构）
int smooth_thr = 0, smooth_pit = 0, smooth_rol = 0, smooth_yaw = 0;

void menu_fps(u8g2_t *u8g2)
{
    static uint32_t last_fps_tick = 0;
    static uint16_t frame_counter = 0;
    static uint8_t current_fps = 0;
    char fps_buf[8];

    frame_counter++;
    uint32_t now = systick_get_ms();

    // 每秒计算一次FPS
    if (now - last_fps_tick >= 1000)
    {
        current_fps = frame_counter;
        frame_counter = 0;
        last_fps_tick = now;
    }

    // 绘制在右下角
    u8g2_SetFont(u8g2, u8g2_font_6x10_tr);
    sprintf(fps_buf, "%dfps", current_fps);
    u8g2_DrawStr(u8g2, 100, 63, fps_buf);
}

void update_smoothed_values(void)
{
    // 指数滤波因子越大越快（0.3为例）
    float alpha = 0.3f;

    smooth_thr = smooth_thr * (1.0f - alpha) + tx.thr * alpha;
    smooth_pit = smooth_pit * (1.0f - alpha) + tx.pit * alpha;
    smooth_rol = smooth_rol * (1.0f - alpha) + tx.rol * alpha;
    smooth_yaw = smooth_yaw * (1.0f - alpha) + tx.yaw * alpha;
}

void DisplayInfo(void)
{
    uint16_t voltage = 0;
    char buffer[16];

    RemoteVoltageDetect(); // 遥控电压检测

    // 锁定 / 解锁  对频完成后显示（参考中心点 (48, 33)）
    if (rxPacketStatus == 1 && pair.step == DONE)
    {
        if (rxPacket[1] == 0)
        {
            u8g2_SetFont(&u8g2, u8g2_font_twelvedings_t_all);
            u8g2_DrawGlyph(&u8g2, 59, 64, 0x004C);
        }
        else if (rxPacket[1] == 1)
        {
            u8g2_SetFont(&u8g2, u8g2_font_twelvedings_t_all);
            u8g2_DrawGlyph(&u8g2, 59, 64, 0x006C);
        }
    }
    u8g2_SetFont(&u8g2, u8g2_font_6x12_tr);
    // --- 遥控电压显示 ---
    u8g2_DrawStr(&u8g2, 0, 19, "R.V:");
    voltage = (uint16_t)(remoteVoltage * 100);
    sprintf(buffer, "%d.%02dV", voltage / 100, voltage % 100);
    u8g2_DrawStr(&u8g2, 24, 18, buffer); // 右侧显示数值

    // --- 飞机电压显示 ---
    u8g2_DrawStr(&u8g2, 75, 19, "F.V:");
    voltage = (rxPacket[4] << 8) | rxPacket[3];
    sprintf(buffer, "%d.%02dV", voltage / 100, voltage % 100);
    u8g2_DrawStr(&u8g2, 99, 18, buffer);

    // 若飞机电压低于 3.4V，显示警告（覆盖原位置）
    if (rxPacketStatus == 1 && pair.step == DONE)
    {
        if (voltage < 340)
        {
            u8g2_SetDrawColor(&u8g2, 0); // 绘制背景色（黑色）
            u8g2_DrawBox(&u8g2, 99, 10, 32, 12);
            u8g2_SetDrawColor(&u8g2, 1); // 恢复前景
            u8g2_DrawStr(&u8g2, 99, 18, "low");
        }
    }
}

void ui_icon(void)
{

    u8g2_SetFont(&u8g2, u8g2_font_waffle_t_all);
    u8g2_DrawGlyph(&u8g2, 0, 8, 0xE30b);   // 遥控图标
    u8g2_DrawGlyph(&u8g2, 117, 8, 0xE21F); // 无信号图标

    u8g2_SetFont(&u8g2, u8g2_font_twelvedings_t_all);
    u8g2_DrawGlyph(&u8g2, 2, 63, 0x0047); // 左下角图标

    // 左下角菜单图标，根据状态不同进行高亮处理
    if (menu_state == MENU_SET_HIGHLIGHT)
    {
        // 画黑色背景框实现“高亮”视觉效果
        u8g2_SetFont(&u8g2, u8g2_font_twelvedings_t_all);
        u8g2_SetDrawColor(&u8g2, 1);
        u8g2_DrawBox(&u8g2, 2, 52, 10, 12); // 背景框（左下角）
        u8g2_SetDrawColor(&u8g2, 0);        // 反色绘制图标
        u8g2_DrawGlyph(&u8g2, 2, 63, 0x0047);
        u8g2_SetDrawColor(&u8g2, 1); // 恢复为正常颜色
    }

    u8 currentSignalStatus = (rxPacketStatus != 0) ? 1 : 0;

    if (currentSignalStatus)
    {
        u8g2_SetFont(&u8g2, u8g2_font_waffle_t_all);
        u8g2_DrawGlyph(&u8g2, 117, 8, 0xE222); // 有信号
        u8g2_SetFont(&u8g2, u8g2_font_wqy12_t_chinese1);
        u8g2_DrawStr(&u8g2, 50, 8, "Online");
    }
    else
    {
        u8g2_DrawGlyph(&u8g2, 117, 8, 0xE21F); // 无信号
        u8g2_SetFont(&u8g2, u8g2_font_wqy12_t_chinese1);
        u8g2_DrawStr(&u8g2, 42, 8, "missing...");
    }
    lastSignalStatus = currentSignalStatus;
}

void no2_menu_ui_icon(void)
{
    
}

void Remotecontroldata(void)
{
    char buffer[16];

    analyze_packet(ADC_value); // 更新tx

    uint8_t bar;
    uint8_t bar_x_T_R = u8g2_GetStrWidth(&u8g2, "T:000") + 5;     // T和R进度条对齐位置
    uint8_t bar_x_P_Y = 70 + u8g2_GetStrWidth(&u8g2, "P:00") + 5; // P和Y进度条对齐位置
    uint8_t max_bar_width = 25;

    // ---------- 第一行：Thr + Pit ----------
    // Thr 油门
    sprintf(buffer, "T:%d", tx.thr);
    u8g2_DrawStr(&u8g2, 0, 30, buffer);

    bar = (smooth_thr > 1000) ? max_bar_width : (smooth_thr * max_bar_width / 1000);
    u8g2_DrawFrame(&u8g2, bar_x_T_R, 22, 28, 8);    // 外框28px
    u8g2_DrawBox(&u8g2, bar_x_T_R + 2, 24, bar, 4); // 内部24px，左右各留2px

    // Pit
    sprintf(buffer, "P:%d", tx.pit);
    u8g2_DrawStr(&u8g2, 70, 30, buffer);

    int16_t pit_offset = (int16_t)tx.pit - 50;
    u8g2_DrawFrame(&u8g2, bar_x_P_Y, 22, 28, 8);
    u8g2_DrawVLine(&u8g2, bar_x_P_Y + 13, 24, 4); // 中线

    if (pit_offset < 0)
    {
        uint8_t width = (-pit_offset) * 11 / 50;
        u8g2_DrawBox(&u8g2, bar_x_P_Y + 13 - width, 24, width, 4);
    }
    else
    {
        uint8_t width = pit_offset * 13 / 50;
        u8g2_DrawBox(&u8g2, bar_x_P_Y + 14, 24, width, 4);
    }

    // ---------- 第二行：Rol + Yaw ----------
    // Rol
    sprintf(buffer, "R:%d", tx.rol);
    u8g2_DrawStr(&u8g2, 0, 44, buffer);

    int16_t rol_offset = (int16_t)tx.rol - 50;
    u8g2_DrawFrame(&u8g2, bar_x_T_R, 36, 28, 8);
    u8g2_DrawVLine(&u8g2, bar_x_T_R + 13, 38, 4); // 中线

    if (rol_offset < 0)
    {
        uint8_t width = (-rol_offset) * 11 / 50;
        u8g2_DrawBox(&u8g2, bar_x_T_R + 13 - width, 38, width, 4);
    }
    else
    {
        uint8_t width = rol_offset * 13 / 50;
        u8g2_DrawBox(&u8g2, bar_x_T_R + 14, 38, width, 4);
    }

    // Yaw
    sprintf(buffer, "Y:%d", tx.yaw);
    u8g2_DrawStr(&u8g2, 70, 44, buffer);

    int16_t yaw_offset = (int16_t)tx.yaw - 50;
    u8g2_DrawFrame(&u8g2, bar_x_P_Y, 36, 28, 8);
    u8g2_DrawVLine(&u8g2, bar_x_P_Y + 13, 38, 4); // 中线

    if (yaw_offset < 0)
    {
        uint8_t width = (-yaw_offset) * 11 / 50;
        u8g2_DrawBox(&u8g2, bar_x_P_Y + 13 - width, 38, width, 4);
    }
    else
    {
        uint8_t width = yaw_offset * 13 / 50;
        u8g2_DrawBox(&u8g2, bar_x_P_Y + 14, 38, width, 4);
    }
}

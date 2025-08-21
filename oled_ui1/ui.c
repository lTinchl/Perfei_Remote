// OLED_ui.c
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
#include "ui.h"
#include "OLED.h"   // 你的OLED接口头文件（确保包含 OLED_Clear/OLED_ShowString/OLED_Update 等）

// ========= 外部符号 =========
extern Pair pair;
extern uint16_t ADC_value[5];

// 图标状态变量
static u8 lastSignalStatus = 0xFF; // 初始化为非法值，确保第一次刷新

extern struct {
    uint8_t Back;
} KeyState;

// ========= 本地状态 =========
static int smooth_thr = 0, smooth_pit = 0, smooth_rol = 0, smooth_yaw = 0;

// 指数平滑
static inline void update_smoothed_values(void)
{
    const float alpha = 0.3f;
    smooth_thr = (int)(smooth_thr * (1.0f - alpha) + tx.thr * alpha);
    smooth_pit = (int)(smooth_pit * (1.0f - alpha) + tx.pit * alpha);
    smooth_rol = (int)(smooth_rol * (1.0f - alpha) + tx.rol * alpha);
    smooth_yaw = (int)(smooth_yaw * (1.0f - alpha) + tx.yaw * alpha);
}

// ========== 画图工具封装（若无矩形API则降级为文本） ==========
static void DrawRectCompat(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
#ifdef OLED_DrawRect
    OLED_DrawRect(x, y, w, h, 1 /*color*/);
#else
    // 无矩形API时，用方括号表示边框
    OLED_ShowString(x, y, "[", OLED_7X12_HALF);
    OLED_ShowString(x + w - 6, y, "]", OLED_7X12_HALF); // 大致占位
#endif
}

static void FillRectCompat(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
#ifdef OLED_FillRect
    OLED_FillRect(x, y, w, h, 1 /*color*/);
#else
    // 用若干 '#' 模拟填充条
    // 每7px 近似一个字符（7x12字库）
    uint8_t chars = (w >= 7) ? (w / 7) : 1;
    char buf[20];
    if (chars > sizeof(buf)-1) chars = sizeof(buf)-1;
    for (uint8_t i = 0; i < chars; ++i) buf[i] = '#';
    buf[chars] = '\0';
    OLED_ShowString(x, y, buf, OLED_7X12_HALF);
#endif
}

static void DrawHBar(uint8_t x, uint8_t y, uint8_t outer_w, uint8_t outer_h, uint8_t inner_w)
{
    if (outer_w < 2 || outer_h < 2) return;
    // 外框
    DrawRectCompat(x, y, outer_w, outer_h);
    // 内条：左右各留1px
    uint8_t max_inner = (outer_w > 2) ? (outer_w - 2) : 0;
    if (inner_w > max_inner) inner_w = max_inner;
    if (inner_w > 0)
        FillRectCompat(x + 1, y + 1, inner_w, (outer_h > 2) ? (outer_h - 2) : 1);
}

// pit/rol/yaw 是以 50 为中点的左右偏移条
static void DrawCenterBar(uint8_t x, uint8_t y, uint8_t outer_w, uint8_t outer_h, int8_t offset /*-50..+50*/)
{
    if (outer_w < 4 || outer_h < 2) return;
    DrawRectCompat(x, y, outer_w, outer_h);
    // 中线
#ifdef OLED_DrawVLine
    OLED_DrawVLine(x + outer_w/2, y + 1, (outer_h > 2) ? (outer_h - 2) : 1, 1);
#else
    OLED_ShowString(x + outer_w/2 - 3, y, "|", OLED_7X12_HALF);
#endif
    // 填充
    uint8_t half_w = (outer_w - 2) / 2; // 去掉左右1px边
    if (offset < 0) {
        // 向左，宽度按 50 -> half_w
        uint8_t w = (uint8_t)((-offset) * half_w / 50);
        if (w) FillRectCompat(x + 1 + (half_w - w), y + 1, w, (outer_h > 2) ? (outer_h - 2) : 1);
    } else if (offset > 0) {
        uint8_t w = (uint8_t)(offset * half_w / 50);
        if (w) FillRectCompat(x + 1 + half_w, y + 1, w, (outer_h > 2) ? (outer_h - 2) : 1);
    }
}

// ========== 文本缓冲 ==========
static char linebuf[24];

// ========== 主页面函数 ==========
void UavInfo_StaticPage(void)
{
    // 进入页面先清屏
    OLED_Clear();

    // ====== 顶部状态栏：在线/离线、锁/解锁 ======
    // Online / missing...
    if (rxPacketStatus != 0) {
        OLED_ShowString(2, 2,  "Online",     OLED_7X12_HALF);
    } else {
        OLED_ShowString(2, 2,  "missing...", OLED_7X12_HALF);
    }

    // 锁/解锁（对频完成后才显示）
    if (rxPacketStatus == 1 && pair.step == DONE) {
        if (rxPacket[1] == 0) {
            OLED_ShowString(92, 2, "[LOCK]",   OLED_7X12_HALF);
        } else if (rxPacket[1] == 1) {
            OLED_ShowString(92, 2, "[UNLOCK]", OLED_7X12_HALF);
        }
    }

    // ====== 电压行（R.V/F.V） ======
    // 遥控电压
    RemoteVoltageDetect();
    uint16_t rv = (uint16_t)(remoteVoltage * 100.0f); // x.xxV -> 整数
    snprintf(linebuf, sizeof(linebuf), "R.V:%d.%02dV", rv/100, rv%100);
    OLED_ShowString(2, 16, linebuf, OLED_7X12_HALF);

    // 飞机电压（来自 rxPacket[4..3] 以 0.01V 标度）
    uint16_t fv_raw = ((uint16_t)rxPacket[4] << 8) | rxPacket[3];
    snprintf(linebuf, sizeof(linebuf), "F.V:%d.%02dV", fv_raw/100, fv_raw%100);
    OLED_ShowString(70, 16, linebuf, OLED_7X12_HALF);

    // 低压告警：F.V < 3.40V
    if (rxPacketStatus == 1 && pair.step == DONE) {
        if (fv_raw < 340) {
#ifdef OLED_FillRect
            // 覆盖原区域并写 "low"
            OLED_FillRect(70, 16, 58, 12, 1);
            OLED_ShowString(70, 16, "low", OLED_7X12_HALF); // 若有反白字接口可改用
#else
            OLED_ShowString(70, 28, "LOW", OLED_7X12_HALF);
#endif
        }
    }

    // ====== 标题 ======
    OLED_ShowString(2, 30, "UAV INFO", OLED_7X12_HALF);

    // ====== 四通道条形图 ======
    // 先解析ADC -> tx
    analyze_packet(ADC_value);
    update_smoothed_values();

    // 位置与尺寸（可根据你屏幕实际调整）
    const uint8_t outer_w = 28, outer_h = 10;
    const uint8_t left_x   = 2 + 7 * 4;   // 左列条形图起点（留出“T:000”文字宽度）
    const uint8_t right_x  = 70 + 7 * 3;  // 右列条形图起点（留出“P:00”文字宽度）
    const uint8_t row1_y   = 42;
    const uint8_t row2_y   = 56;

    // --- T（0~1000） & P（0~100 中点50）---
    snprintf(linebuf, sizeof(linebuf), "T:%d", tx.thr);
    OLED_ShowString(2, row1_y, linebuf, OLED_7X12_HALF);
    {
        uint8_t max_bar_w = (outer_w > 2) ? (outer_w - 2) : outer_w;
        uint8_t inner = (smooth_thr > 1000) ? max_bar_w
                         : (uint8_t)((smooth_thr * max_bar_w) / 1000);
        DrawHBar(left_x, row1_y - 2, outer_w, outer_h, inner);
    }

    snprintf(linebuf, sizeof(linebuf), "P:%d", tx.pit);
    OLED_ShowString(70, row1_y, linebuf, OLED_7X12_HALF);
    {
        int16_t off = (int16_t)smooth_pit - 50; // -50..+50
        DrawCenterBar(right_x, row1_y - 2, outer_w, outer_h, (int8_t)off);
    }

    // --- R & Y ---
    snprintf(linebuf, sizeof(linebuf), "R:%d", tx.rol);
    OLED_ShowString(2, row2_y, linebuf, OLED_7X12_HALF);
    {
        int16_t off = (int16_t)smooth_rol - 50;
        DrawCenterBar(left_x, row2_y - 2, outer_w, outer_h, (int8_t)off);
    }

    snprintf(linebuf, sizeof(linebuf), "Y:%d", tx.yaw);
    OLED_ShowString(70, row2_y, linebuf, OLED_7X12_HALF);
    {
        int16_t off = (int16_t)smooth_yaw - 50;
        DrawCenterBar(right_x, row2_y - 2, outer_w, outer_h, (int8_t)off);
    }

    // ====== 底部返回提示 ======
#ifdef OLED_FillRect
    // 若有反白矩形，可画底条
    // OLED_FillRect(0, 56, 128, 8, 1);
#endif
    OLED_ShowString(4, 60, "[back]", OLED_7X12_HALF);

    // 先刷新一次
    OLED_Update();

    // ====== 页面停留循环：动态刷新与退出 ======
    // 如需更平滑，可加上定时（比如每20~50ms刷新）
    while (1)
    {
        // 退出
        if (KeyState.Back) {
            OLED_UI_Back();
            return;
        }

        // 可选：周期性刷新动态数据（电压/通道）
        RemoteVoltageDetect();
        analyze_packet(ADC_value);
        update_smoothed_values();

        // 局部重绘（简单处理：直接全部重画，方便稳定）
        OLED_Clear();

        // 顶部状态
        if (rxPacketStatus != 0) {
            OLED_ShowString(2, 2,  "Online",     OLED_7X12_HALF);
        } else {
            OLED_ShowString(2, 2,  "missing...", OLED_7X12_HALF);
        }
        if (rxPacketStatus == 1 && pair.step == DONE) {
            if (rxPacket[1] == 0) OLED_ShowString(92, 2, "[LOCK]",   OLED_7X12_HALF);
            else                  OLED_ShowString(92, 2, "[UNLOCK]", OLED_7X12_HALF);
        }

        // 电压
        rv = (uint16_t)(remoteVoltage * 100.0f);
        snprintf(linebuf, sizeof(linebuf), "R.V:%d.%02dV", rv/100, rv%100);
        OLED_ShowString(2, 16, linebuf, OLED_7X12_HALF);

        fv_raw = ((uint16_t)rxPacket[4] << 8) | rxPacket[3];
        snprintf(linebuf, sizeof(linebuf), "F.V:%d.%02dV", fv_raw/100, fv_raw%100);
        OLED_ShowString(70, 16, linebuf, OLED_7X12_HALF);
        if (rxPacketStatus == 1 && pair.step == DONE && fv_raw < 340) {
#ifdef OLED_FillRect
            OLED_FillRect(70, 16, 58, 12, 1);
            OLED_ShowString(70, 16, "low", OLED_7X12_HALF);
#else
            OLED_ShowString(70, 28, "LOW", OLED_7X12_HALF);
#endif
        }

        // 标题
        OLED_ShowString(2, 30, "UAV INFO", OLED_7X12_HALF);

        // 四通道
        snprintf(linebuf, sizeof(linebuf), "T:%d", tx.thr);
        OLED_ShowString(2, row1_y, linebuf, OLED_7X12_HALF);
        {
            uint8_t max_bar_w = (outer_w > 2) ? (outer_w - 2) : outer_w;
            uint8_t inner = (smooth_thr > 1000) ? max_bar_w
                             : (uint8_t)((smooth_thr * max_bar_w) / 1000);
            DrawHBar(left_x, row1_y - 2, outer_w, outer_h, inner);
        }

        snprintf(linebuf, sizeof(linebuf), "P:%d", tx.pit);
        OLED_ShowString(70, row1_y, linebuf, OLED_7X12_HALF);
        DrawCenterBar(right_x, row1_y - 2, outer_w, outer_h, (int8_t)((int16_t)smooth_pit - 50));

        snprintf(linebuf, sizeof(linebuf), "R:%d", tx.rol);
        OLED_ShowString(2, row2_y, linebuf, OLED_7X12_HALF);
        DrawCenterBar(left_x,  row2_y - 2, outer_w, outer_h, (int8_t)((int16_t)smooth_rol - 50));

        snprintf(linebuf, sizeof(linebuf), "Y:%d", tx.yaw);
        OLED_ShowString(70, row2_y, linebuf, OLED_7X12_HALF);
        DrawCenterBar(right_x, row2_y - 2, outer_w, outer_h, (int8_t)((int16_t)smooth_yaw - 50));

        OLED_ShowString(4, 60, "[back]", OLED_7X12_HALF);

        OLED_Update();

        // 简单节流：如果你有 systick，可加个10~30ms 延时
         delay_ms(30);
    }
}

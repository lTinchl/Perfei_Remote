#include "u8g2.h"
#include "u8g2_OLEDinit.h"
#include "stm32f10x.h"
#include "IIC_OLED.h"
#include "systick.h"

//u8g2_t u8g2;

//uint8_t u8g2_gpio_and_delay_stm32(U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr)
//{
//	switch (msg)
//	{
//	// Function which delays ms	
//	case U8X8_MSG_DELAY_MILLI:
//		delay_ms(arg_int);
//		break;
//	// Function which delays 10us
//	case U8X8_MSG_DELAY_10MICRO:
//		delay_us(10);
//		break;
//	// Function which delays 100ns
//	case U8X8_MSG_DELAY_100NANO:
//		__NOP();
//		break;
//	case U8X8_MSG_GPIO_I2C_CLOCK:
//		if (arg_int)
//			IIC_OLED_SCL_HIGH();
//		else
//			IIC_OLED_SCL_LOW();
//		break;
//	case U8X8_MSG_GPIO_I2C_DATA:
//		if (arg_int)
//			IIC_OLED_SDA_HIGH();
//		else
//			IIC_OLED_SDA_LOW();
//		break;
//	default:
//		return 0; // A message was received which is not implemented, return 0 to indicate an error
//	}
//	return 1; // command processed successfully.
//}

//void u8g2_OLED_Init_cn1(void) {
//    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8g2_gpio_and_delay_stm32);
//    u8g2_InitDisplay(&u8g2);
//    u8g2_SetPowerSave(&u8g2, 0);
//    u8g2_SetFont(&u8g2, u8g2_font_unifont_t_chinese1); // 中文字体
//}

//void u8g2_OLED_Init_cn2(void) {
//    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8g2_gpio_and_delay_stm32);
//    u8g2_InitDisplay(&u8g2);
//    u8g2_SetPowerSave(&u8g2, 0);
//    u8g2_SetFont(&u8g2, u8g2_font_unifont_t_chinese2); // 中文字体
//}

//void u8g2_OLED_Init_cn3(void) {
//    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8g2_gpio_and_delay_stm32);
//    u8g2_InitDisplay(&u8g2);
//    u8g2_SetPowerSave(&u8g2, 0);
//    u8g2_SetFont(&u8g2, u8g2_font_unifont_t_chinese3); // 中文字体
//}


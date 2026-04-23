#include "bsp_lcd.h"
#include <stdio.h>

/**
 * @brief  显示BMP图片（统一接口）
 * @param  x: 显示X坐标
 * @param  y: 显示Y坐标
 * @param  filename: BMP文件名
 * @retval 0:成功, 其他:失败
 */
uint8_t LCD_ShowBMP(uint16_t x, uint16_t y, const char* filename)
{
    return Lcd_Show_bmp(x, y, filename);
}
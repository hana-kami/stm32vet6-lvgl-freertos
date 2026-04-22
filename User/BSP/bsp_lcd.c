#include "bsp_lcd.h"
#include <stdio.h>
#include <string.h>

/* 静态变量 */
static uint16_t g_lcd_width = LCD_WIDTH;
static uint16_t g_lcd_height = LCD_HEIGHT;
static uint8_t g_lcd_direction = LCD_DIRECTION;

/* ==================== 基础图形函数 ==================== */

/**
 * @brief  LCD 初始化
 */
void LCD_Init(void)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_Init();
    // printf("ILI9341 LCD初始化完成: %dx%d\n", g_lcd_width, g_lcd_height);
#endif
}

/**
 * @brief  全屏清屏
 */
void LCD_Clear(uint16_t color)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_Clear(0, 0, g_lcd_width, g_lcd_height, color);
#endif
}

/**
 * @brief  区域清屏
 */
void LCD_ClearArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_Clear(x, y, width, height, color);
#endif
}

/**
 * @brief  画点
 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if(x >= g_lcd_width || y >= g_lcd_height) return;
    
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_SetPointPixel(x, y, color);
#endif

}

/**
 * @brief  获取点颜色
 */
uint16_t LCD_GetPoint(uint16_t x, uint16_t y)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    return ILI9341_GetPointPixel(x, y);
#elif (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341_QSPI)
    ILI9341_QSPI_DrawPoint(x, y, color);
#endif
}

/**
 * @brief  画线
 */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_DrawLine(x1, y1, x2, y2, color);
#endif
}

/**
 * @brief  画矩形
 */
void LCD_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, 
                       uint16_t color, uint8_t filled)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_DrawRectangle(x, y, width, height, color, filled);
#endif
}

/**
 * @brief  画圆
 */
void LCD_DrawCircle(uint16_t xc, uint16_t yc, uint16_t radius, uint16_t color, uint8_t filled)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_DrawCircle(xc, yc, radius, color, filled);
#endif
}

/* ==================== 文字显示函数 ==================== */

/**
 * @brief  显示英文字符
 */
void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t bg_color, uint16_t fg_color)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_DispChar_EN(x, y, ch, bg_color, fg_color);
#endif
}

/**
 * @brief  显示英文字符串
 */
void LCD_ShowString(uint16_t x, uint16_t y, const char* str, uint16_t bg_color, uint16_t fg_color)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_DispString_EN(x, y, str, bg_color, fg_color);
#endif
}

/**
 * @brief  显示中文字符
 */
void LCD_ShowChinese(uint16_t x, uint16_t y, uint16_t ch, uint16_t bg_color, uint16_t fg_color)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_DispChar_CH(x, y, ch, bg_color, fg_color);
#endif
}

/**
 * @brief  显示中文字符串
 */
void LCD_ShowChineseString(uint16_t x, uint16_t y, const uint8_t* str, 
                           uint16_t bg_color, uint16_t fg_color)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_DispString_CH(x, y, str, bg_color, fg_color);
#endif
}

/**
 * @brief  显示中英文混合字符串
 */
void LCD_ShowStringMixed(uint16_t x, uint16_t y, const uint8_t* str, 
                         uint16_t bg_color, uint16_t fg_color)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_DispString_EN_CH(x, y, str, bg_color, fg_color);
#endif
}

/**
 * @brief  显示数字
 */
void LCD_ShowNumber(uint16_t x, uint16_t y, uint32_t num, uint8_t len, 
                    uint16_t bg_color, uint16_t fg_color)
{
    char buf[20];
    sprintf(buf, "%0*d", len, num);
    LCD_ShowString(x, y, buf, bg_color, fg_color);
}

/* ==================== 高级功能函数 ==================== */

/**
 * @brief  显示BMP图片
 */
uint8_t LCD_ShowBMP(uint16_t x, uint16_t y, const char* filename)
{
    // 调用你的BMP显示函数
    extern void Lcd_Show_bmp(uint16_t x, uint16_t y, const char* filename);
    Lcd_Show_bmp(x, y, (char*)filename);
    return 0;
}

/**
 * @brief  设置GRAM扫描方向
 */
void LCD_SetScanDirection(uint8_t direction)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_GramScan(direction);
    
    // 根据扫描方向更新宽高
#endif
}

/**
 * @brief  打开显示窗口
 */
void LCD_SetWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_OpenWindow(x, y, width, height);
#endif
}

/**
 * @brief  背光控制
 */
void LCD_BacklightControl(uint8_t on)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_BackLed_Control(on ? ENABLE : DISABLE);
#endif
}

/**
 * @brief  获取屏幕宽度
 */
uint16_t LCD_GetWidth(void)
{
    return g_lcd_width;
}

/**
 * @brief  获取屏幕高度
 */
uint16_t LCD_GetHeight(void)
{
    return g_lcd_height;
}

/**
 * @brief  显示灰度测试图案
 */
void LCD_ShowGrayPattern(void)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    disp_gray();
#endif
}

/**
 * @brief  显示9宫格棋盘格测试图案
 */
void LCD_ShowChessboard9(void)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    DISP9W();
#endif
}

/**
 * @brief  LVGL图形库刷新接口
 */
void LCD_LVGL_Flush(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t* colors)
{
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    ILI9341_LVGL_Flush(x, y, width, height, colors);
#endif
}
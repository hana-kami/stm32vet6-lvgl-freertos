#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include <stdint.h>
#include "stm32f10x.h"
#include "bsp_ili9341_lcd.h"

/* ==================== 屏幕类型选择 ==================== */
// 当前使用的屏幕类型
#define LCD_TYPE_ILI9341    1   // ILI9341 屏幕 (240x320)
#define LCD_TYPE_ST7789     2   // ST7789 屏幕 (240x240) - 预留
#define LCD_TYPE_SSD1306    3   // SSD1306 OLED (128x64) - 预留

// 修改这里来切换屏幕类型
#define CURRENT_LCD_TYPE    LCD_TYPE_ILI9341

/* ==================== 屏幕尺寸定义 ==================== */
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    #define LCD_WIDTH       240
    #define LCD_HEIGHT      320
    #define LCD_DIRECTION   1   // 1:竖屏, 0:横屏
    
#elif (CURRENT_LCD_TYPE == LCD_TYPE_ST7789)
    #define LCD_WIDTH       240
    #define LCD_HEIGHT      240
    #define LCD_DIRECTION   1
    
#elif (CURRENT_LCD_TYPE == LCD_TYPE_SSD1306)
    #define LCD_WIDTH       128
    #define LCD_HEIGHT      64
    #define LCD_DIRECTION   0
    
#else
    #error "No LCD type selected!"
#endif

/* ==================== 颜色定义（统一RGB565格式） ==================== */
#define LCD_COLOR_WHITE     macWHITE
#define LCD_COLOR_BLACK     macBLACK
#define LCD_COLOR_RED       macRED
#define LCD_COLOR_GREEN     macGREEN
#define LCD_COLOR_BLUE      macBLUE
#define LCD_COLOR_YELLOW    macYELLOW
#define LCD_COLOR_CYAN      macCYAN
#define LCD_COLOR_MAGENTA   macMAGENTA
#define LCD_COLOR_GRAY      macGREY

/* ==================== API 函数声明 ==================== */

/**
 * @brief  LCD 初始化（统一入口）
 */
void LCD_Init(void);

/**
 * @brief  清屏（全屏）
 * @param  color: 填充颜色
 */
void LCD_Clear(uint16_t color);

/**
 * @brief  区域清屏填充
 * @param  x,y: 起始坐标
 * @param  width,height: 宽度和高度
 * @param  color: 填充颜色
 */
void LCD_ClearArea(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

/**
 * @brief  画点
 * @param  x,y: 坐标
 * @param  color: 颜色
 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief  获取点颜色
 * @param  x,y: 坐标
 * @retval 颜色值
 */
uint16_t LCD_GetPoint(uint16_t x, uint16_t y);

/**
 * @brief  画线
 * @param  x1,y1: 起点坐标
 * @param  x2,y2: 终点坐标
 * @param  color: 颜色
 */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

/**
 * @brief  画矩形
 * @param  x,y: 起始坐标
 * @param  width,height: 宽度和高度
 * @param  color: 颜色
 * @param  filled: 是否填充 (0:边框, 1:填充)
 */
void LCD_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, 
                       uint16_t color, uint8_t filled);

/**
 * @brief  画圆
 * @param  xc,yc: 圆心坐标
 * @param  radius: 半径
 * @param  color: 颜色
 * @param  filled: 是否填充
 */
void LCD_DrawCircle(uint16_t xc, uint16_t yc, uint16_t radius, uint16_t color, uint8_t filled);

/**
 * @brief  显示英文字符
 * @param  x,y: 坐标
 * @param  ch: 字符
 * @param  bg_color: 背景色
 * @param  fg_color: 前景色
 */
void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t bg_color, uint16_t fg_color);

/**
 * @brief  显示英文字符串
 * @param  x,y: 坐标
 * @param  str: 字符串
 * @param  bg_color: 背景色
 * @param  fg_color: 前景色
 */
void LCD_ShowString(uint16_t x, uint16_t y, const char* str, uint16_t bg_color, uint16_t fg_color);

/**
 * @brief  显示中文字符
 * @param  x,y: 坐标
 * @param  ch: 中文字符内码
 * @param  bg_color: 背景色
 * @param  fg_color: 前景色
 */
void LCD_ShowChinese(uint16_t x, uint16_t y, uint16_t ch, uint16_t bg_color, uint16_t fg_color);

/**
 * @brief  显示中文字符串
 * @param  x,y: 坐标
 * @param  str: 字符串
 * @param  bg_color: 背景色
 * @param  fg_color: 前景色
 */
void LCD_ShowChineseString(uint16_t x, uint16_t y, const uint8_t* str, 
                           uint16_t bg_color, uint16_t fg_color);

/**
 * @brief  显示中英文混合字符串
 * @param  x,y: 坐标
 * @param  str: 字符串
 * @param  bg_color: 背景色
 * @param  fg_color: 前景色
 */
void LCD_ShowStringMixed(uint16_t x, uint16_t y, const uint8_t* str, 
                         uint16_t bg_color, uint16_t fg_color);

/**
 * @brief  显示数字
 * @param  x,y: 坐标
 * @param  num: 数字
 * @param  len: 位数（不足补0）
 * @param  bg_color: 背景色
 * @param  fg_color: 前景色
 */
void LCD_ShowNumber(uint16_t x, uint16_t y, uint32_t num, uint8_t len, 
                    uint16_t bg_color, uint16_t fg_color);

/**
 * @brief  显示BMP图片
 * @param  x,y: 显示坐标
 * @param  filename: 文件名（如 "/test.bmp"）
 * @retval 0:成功, 其他:失败
 */
uint8_t LCD_ShowBMP(uint16_t x, uint16_t y, const char* filename);

/**
 * @brief  设置GRAM扫描方向
 * @param  direction: 扫描方向 (0-7)
 */
void LCD_SetScanDirection(uint8_t direction);

/**
 * @brief  打开显示窗口
 * @param  x,y: 起始坐标
 * @param  width,height: 宽度和高度
 */
void LCD_SetWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

/**
 * @brief  背光控制
 * @param  on: 1-开背光, 0-关背光
 */
void LCD_BacklightControl(uint8_t on);

/**
 * @brief  获取屏幕宽度
 */
uint16_t LCD_GetWidth(void);

/**
 * @brief  获取屏幕高度
 */
uint16_t LCD_GetHeight(void);

/**
 * @brief  显示灰度测试图案
 */
void LCD_ShowGrayPattern(void);

/**
 * @brief  显示9宫格棋盘格测试图案
 */
void LCD_ShowChessboard9(void);

/**
 * @brief  LVGL图形库刷新接口
 * @param  x,y: 起始坐标
 * @param  width,height: 区域大小
 * @param  colors: 颜色数据
 */
void LCD_LVGL_Flush(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t* colors);

#endif /* __BSP_LCD_H */
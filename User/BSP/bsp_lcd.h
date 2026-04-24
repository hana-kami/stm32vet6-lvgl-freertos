#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include <stdint.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "bsp_lcd_config.h"

/* ==================== 条件包含驱动头文件 ==================== */
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    #include "bsp_ili9341_lcd.h"
    #define LCD_DRIVE_NAME "ILI9341"
#elif (CURRENT_LCD_TYPE == LCD_TYPE_GC9B71)
    #include "bsp_gc9b71_lcd.h"
    #define LCD_DRIVE_NAME "GC9B71"
#elif (CURRENT_LCD_TYPE == LCD_TYPE_ST75161)
    #include "bsp_st75161_lcd.h"
    #define LCD_DRIVE_NAME "ST75161"
#else
    #error "No LCD type selected!"
#endif

/* ==================== 尺寸定义 ==================== */
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    #define LCD_WIDTH       240
    #define LCD_HEIGHT      320
#elif (CURRENT_LCD_TYPE == LCD_TYPE_GC9B71)
    #define LCD_WIDTH       240
    #define LCD_HEIGHT      240
#elif (CURRENT_LCD_TYPE == LCD_TYPE_ST75161)
    #define LCD_WIDTH       160
    #define LCD_HEIGHT      160
#endif

#define LCD_DIRECTION   0

/* ==================== 颜色定义 ==================== */
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    #define LCD_COLOR_WHITE     macWHITE
    #define LCD_COLOR_BLACK     macBLACK
    #define LCD_COLOR_RED       macRED
    #define LCD_COLOR_GREEN     macGREEN
    #define LCD_COLOR_BLUE      macBLUE
    #define LCD_COLOR_YELLOW    macYELLOW
    #define LCD_COLOR_CYAN      macCYAN
    #define LCD_COLOR_MAGENTA   macMAGENTA
    #define LCD_COLOR_GRAY      macGREY

#elif (CURRENT_LCD_TYPE == LCD_TYPE_GC9B71)
    #define LCD_COLOR_WHITE     macWHITE
    #define LCD_COLOR_BLACK     macBLACK
    #define LCD_COLOR_RED       macRED
    #define LCD_COLOR_GREEN     macGREEN
    #define LCD_COLOR_BLUE      macBLUE
    #define LCD_COLOR_YELLOW    macYELLOW
    #define LCD_COLOR_CYAN      macCYAN
    #define LCD_COLOR_MAGENTA   macMAGENTA
    #define LCD_COLOR_GRAY      macGREY

#elif (CURRENT_LCD_TYPE == LCD_TYPE_ST75161)
    #define LCD_COLOR_WHITE     macWHITE
    #define LCD_COLOR_BLACK     macBLACK
    #define LCD_COLOR_RED       macRED
    #define LCD_COLOR_GREEN     macGREEN
    #define LCD_COLOR_BLUE      macBLUE
    #define LCD_COLOR_YELLOW    macYELLOW
    #define LCD_COLOR_CYAN      macCYAN
    #define LCD_COLOR_MAGENTA   macMAGENTA
    #define LCD_COLOR_GRAY      macGREY
#endif

/* ==================== API 宏映射（直接映射到底层函数） ==================== */
#if (CURRENT_LCD_TYPE == LCD_TYPE_ILI9341)
    // ILI9341 驱动映射
    #define LCD_Init                ILI9341_Init
    #define LCD_ClearArea(x,y,w,h,c) ILI9341_Clear(x,y,w,h,c)
    #define LCD_Clear(c)            ILI9341_Clear(0, 0, LCD_WIDTH, LCD_HEIGHT, c)
    #define LCD_DrawPoint           ILI9341_SetPointPixel
    #define LCD_GetPoint            ILI9341_GetPointPixel
    #define LCD_DrawLine            ILI9341_DrawLine
    #define LCD_DrawRectangle       ILI9341_DrawRectangle
    #define LCD_DrawCircle          ILI9341_DrawCircle
    #define LCD_ShowChar            ILI9341_DispChar_EN
    #define LCD_ShowString          ILI9341_DispString_EN
    #define LCD_ShowChinese         ILI9341_DispChar_CH
    #define LCD_ShowChineseString   ILI9341_DispString_CH
    #define LCD_ShowStringMixed     ILI9341_DispString_EN_CH
    #define LCD_SetScanDirection    ILI9341_GramScan
    #define LCD_SetWindow           ILI9341_OpenWindow
    #define LCD_COMMOUT             comm_out
    #define LCD_BacklightControl(on) ILI9341_BackLed_Control(on?ENABLE:DISABLE)
    #define LCD_ShowGrayPattern     disp_gray
    #define LCD_ShowChessboard9     DISP9W
    #define LCD_ShowPicture         pic_show
    #define LCD_LVGL_Flush          ILI9341_LVGL_Flush
    #define LCD_Write_Data          ILI9341_Write_Data
    
#elif (CURRENT_LCD_TYPE == LCD_TYPE_GC9B71)
    // GC9B71 驱动映射
    #define LCD_Init                GC9B71_Init
    #define LCD_ClearArea(x,y,w,h,c) GC9B71_Clear(x,y,w,h,(uint32_t)c)
    #define LCD_Clear(c)            GC9B71_Clear(0, 0, LCD_WIDTH, LCD_HEIGHT, (uint32_t)c)
    #define LCD_DrawPoint           GC9B71_SetPointPixel
    #define LCD_GetPoint            GC9B71_GetPointPixel
    #define LCD_DrawLine            GC9B71_DrawLine
    #define LCD_DrawRectangle       GC9B71_DrawRectangle
    #define LCD_DrawCircle          GC9B71_DrawCircle
    #define LCD_ShowChar            GC9B71_DispChar_EN
    #define LCD_ShowString          GC9B71_DispString_EN
    #define LCD_ShowChinese         GC9B71_DispChar_CH
    #define LCD_ShowChineseString   GC9B71_DispString_CH
    #define LCD_ShowStringMixed     GC9B71_DispString_EN_CH
    #define LCD_SetScanDirection    GC9B71_GramScan
    #define LCD_SetWindow           GC9B71_OpenWindow
    #define LCD_COMMOUT             comm_out
    #define LCD_BacklightControl(on) GC9B71_BackLed_Control(on?ENABLE:DISABLE)
    #define LCD_ShowGrayPattern     disp_gray
    #define LCD_ShowChessboard9     DispFrame
    #define LCD_ShowPicture         pic_show
    #define LCD_Write_Data          GC9B71_Write_Data

#elif (CURRENT_LCD_TYPE == LCD_TYPE_ST75161)
    // ST75161 驱动映射
    #define LCD_Init                ST75161_Init
    #define LCD_ClearArea(x,y,w,h,c) ST75161_Clear(x,y,w,h,c)
    #define LCD_Clear(c)            ST75161_Clear(0, 0, LCD_WIDTH, LCD_HEIGHT, c)
    #define LCD_DrawPoint           ST75161_SetPointPixel
    #define LCD_GetPoint            ST75161_GetPointPixel
    #define LCD_DrawLine            ST75161_DrawLine
    #define LCD_DrawRectangle       ST75161_DrawRectangle
    #define LCD_DrawCircle          ST75161_DrawCircle
    #define LCD_ShowChar            ST75161_DispChar_EN
    #define LCD_ShowString          ST75161_DispString_EN
    #define LCD_ShowChinese         ST75161_DispChar_CH
    #define LCD_ShowChineseString   ST75161_DispString_CH
    #define LCD_ShowStringMixed     ST75161_DispString_EN_CH
    #define LCD_SetScanDirection    ST75161_GramScan
    #define LCD_SetWindow           ST75161_OpenWindow
    #define LCD_COMMOUT             comm_out
    #define LCD_BacklightControl(on) ST75161_BackLed_Control(on?ENABLE:DISABLE)
    #define LCD_ShowGrayPattern     disp_gray
    #define LCD_ShowChessboard9     DISP9W      // ST75161使用9宫格图案
    #define LCD_ShowPicture         pic_show
    #define LCD_Write_Data          ST75161_Write_Data
    // ST75161 暂不支持 LVGL 刷新
#endif

/* ==================== 内联函数定义（必须在头文件中） ==================== */

/**
 * @brief  获取屏幕宽度
 * @retval 屏幕宽度（像素）
 */
static __inline uint16_t LCD_GetWidth(void)
{
    return LCD_WIDTH;
}

/**
 * @brief  获取屏幕高度
 * @retval 屏幕高度（像素）
 */
static __inline uint16_t LCD_GetHeight(void)
{
    return LCD_HEIGHT;
}

/**
 * @brief  显示数字（需要格式化，不能用简单的宏映射）
 * @param  x: X坐标
 * @param  y: Y坐标
 * @param  num: 要显示的数字
 * @param  len: 显示位数（不足补0）
 * @param  bg_color: 背景色
 * @param  fg_color: 前景色
 * @retval 无
 */
static __inline void LCD_ShowNumber(uint16_t x, uint16_t y, uint32_t num, uint8_t len, 
                                     uint16_t bg_color, uint16_t fg_color)
{
    char buf[20];
    sprintf(buf, "%0*d", len, num);
    LCD_ShowString(x, y, buf, bg_color, fg_color);
}

/**
 * @brief  显示BMP图片（需要文件系统支持）
 * @param  x: 显示X坐标
 * @param  y: 显示Y坐标
 * @param  filename: BMP文件名
 * @retval 0:成功, 其他:失败
 */
uint8_t LCD_ShowBMP(uint16_t x, uint16_t y, const char* filename);

#endif /* __BSP_LCD_H */


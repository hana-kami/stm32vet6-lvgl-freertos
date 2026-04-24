#ifndef __BSP_LCD_CONFIG_H
#define __BSP_LCD_CONFIG_H

/* ==================== 屏幕类型选择 ==================== */
#define LCD_TYPE_ILI9341          1
#define LCD_TYPE_GC9B71           2
#define LCD_TYPE_ST75161          3   // ST75161 单色/灰度LCD屏幕

// 修改这里来切换屏幕类型
#define CURRENT_LCD_TYPE    LCD_TYPE_ILI9341   // 当前使用ST75161屏幕

/* ==================== 颜色转换宏 ==================== */
/**
 * @brief RGB24格式转RGB16格式
 * @param R 红色分量（0-255）
 * @param G 绿色分量（0-255）
 * @param B 蓝色分量（0-255）
 * @retval RGB565格式的颜色值（16位）
 */
#define RGB24TORGB16(R,G,B) ((uint16_t)((((R)>>3)<<11) | (((G)>>2)<<5) | ((B)>>3)))

#endif /* __BSP_LCD_CONFIG_H */
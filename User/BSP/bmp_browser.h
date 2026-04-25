#ifndef __BMP_BROWSER_H
#define __BMP_BROWSER_H

#include <stdint.h>

/**
 * @brief  初始化BMP浏览器
 */
void BMP_Browser_Init(void);

/**
 * @brief  获取BMP文件总数
 * @retval 文件数量
 */
uint16_t BMP_Browser_GetTotalCount(void);

/**
 * @brief  显示当前图片
 * @param  x, y 显示坐标
 * @retval 1:成功, 0:失败
 */
uint8_t BMP_Browser_ShowCurrent(uint16_t x, uint16_t y);

/**
 * @brief  下一张图片
 */
void BMP_Browser_Next(void);

/**
 * @brief  上一张图片
 */
void BMP_Browser_Prev(void);

/**
 * @brief  跳转到指定索引
 * @param  index 目标索引
 */
void BMP_Browser_Goto(uint16_t index);

/**
 * @brief  刷新文件列表（重新扫描文件数量）
 */
void BMP_Browser_Refresh(void);

/**
 * @brief  获取当前索引
 * @retval 当前索引
 */
uint16_t BMP_Browser_GetCurrentIndex(void);

#endif
#ifndef __ANIMATION_FRAMES_H
#define __ANIMATION_FRAMES_H

#include "lvgl.h"

/* 使用Image2Lcd工具将BMP转换为C数组
 * 配置参数：
 * - 输出格式：C语言数组
 * - 颜色格式：RGB565
 * - 扫描模式：水平扫描
 * - 高位在前：是
 * - 包含图像头数据：否
 */


/* 帧0 */
extern const lv_img_dsc_t frame_0;
// /* 帧1 */
// extern const lv_img_dsc_t frame_1;
// /* 帧2 */
// extern const lv_img_dsc_t frame_2;

// /* 帧数组 */
// extern const lv_img_dsc_t* frames[];

/* 总帧数 */
#define TOTAL_FRAMES 1

#endif

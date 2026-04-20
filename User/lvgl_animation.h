#ifndef __LVGL_ANIMATION_H
#define __LVGL_ANIMATION_H

#include "lvgl.h"

#define ANIMATION_TOTAL_FRAMES    5    // 总帧数
#define ANIMATION_DEFAULT_DELAY   100   // 默认100ms/帧 (10fps)
#define ANIMATION_LOOP_ENABLE     1     // 循环播放

/* API函数 */
void LVGL_Animation_Init(void);
void LVGL_Animation_Stop(void);
void LVGL_Animation_Start(void);
void LVGL_Animation_SetSpeed(uint32_t delay_ms);
void LVGL_Animation_SetFrame(uint8_t frame_index);
uint8_t LVGL_Animation_GetCurrentFrame(void);
void LVGL_Animation_Pause(void);
void LVGL_Animation_Resume(void);

/* 外部帧数组 - 你需要在实际文件中定义 */
extern const lv_img_dsc_t* frames[];

#endif

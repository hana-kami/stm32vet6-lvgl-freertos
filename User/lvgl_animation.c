#include "lvgl_animation.h"
#include "animation_frames.h" 

/* 静态变量 */
static lv_obj_t* anim_img = NULL;
static lv_timer_t* anim_timer = NULL;
static uint8_t current_frame = 0;
static uint8_t is_paused = 0;
static uint32_t frame_delay = ANIMATION_DEFAULT_DELAY;

/* 定时器回调函数 */
static void animation_timer_cb(lv_timer_t* timer)
{
    if(anim_img == NULL || is_paused) return;
    
    current_frame++;
    if(current_frame >= ANIMATION_TOTAL_FRAMES) {
#if ANIMATION_LOOP_ENABLE
        current_frame = 0;  // 循环播放
#else
        // 非循环模式，停止定时器
        lv_timer_pause(anim_timer);
        return;
#endif
    }
    
    /* 设置图片源 */
    lv_img_set_src(anim_img, frames[current_frame]);
    
    /* 可选：添加淡入淡出效果 */
    // lv_anim_t a;
    // lv_anim_init(&a);
    // lv_anim_set_var(&a, anim_img);
    // lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    // lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    // lv_anim_set_time(&a, frame_delay / 2);
    // lv_anim_start(&a);
}

/* 初始化LVGL动画 */
void LVGL_Animation_Init(void)
{
    /* 创建图片控件 */
    anim_img = lv_img_create(lv_scr_act());
    if(anim_img == NULL) {
        printf("LVGL动画：创建图片控件失败！\n");
        return;
    }
    
    /* 设置图片位置
    lv_obj_set_pos(anim_img, 0, 0);
    // lv_obj_center(anim_img);  // 居中显示
    
    /* 设置图片大小（可选，会缩放图片） */
    // lv_obj_set_size(anim_img, 320, 240);
    
    /* 设置第一帧 */
    if(frames[0] != NULL) {
        lv_img_set_src(anim_img, frames[0]);
    } else {
        printf("LVGL动画：第一帧数据为空！\n");
    }
    
    /* 创建定时器 */
    anim_timer = lv_timer_create(animation_timer_cb, frame_delay, NULL);
    if(anim_timer == NULL) {
        printf("LVGL动画：创建定时器失败！\n");
    }
    
    current_frame = 0;
    is_paused = 0;
    
    printf("LVGL动画已启动，总帧数：%d，帧延迟：%dms\n", 
           ANIMATION_TOTAL_FRAMES, frame_delay);
}

/* 停止动画并释放资源 */
void LVGL_Animation_Stop(void)
{
    if(anim_timer != NULL) {
        lv_timer_del(anim_timer);
        anim_timer = NULL;
    }
    
    if(anim_img != NULL) {
        lv_obj_del(anim_img);
        anim_img = NULL;
    }
    
    current_frame = 0;
    is_paused = 0;
    
    printf("LVGL动画已停止\n");
}

/* 启动动画 */
void LVGL_Animation_Start(void)
{
    if(anim_timer != NULL && is_paused) {
        lv_timer_resume(anim_timer);
        is_paused = 0;
        printf("LVGL动画已恢复\n");
    } else if(anim_timer == NULL) {
        // 如果定时器不存在，重新初始化
        LVGL_Animation_Init();
    }
}

/* 设置动画速度 */
void LVGL_Animation_SetSpeed(uint32_t delay_ms)
{
    frame_delay = delay_ms;
    
    if(anim_timer != NULL) {
        lv_timer_set_period(anim_timer, frame_delay);
        printf("LVGL动画速度已设置为 %dms/帧\n", frame_delay);
    }
}

/* 手动设置指定帧 */
void LVGL_Animation_SetFrame(uint8_t frame_index)
{
    if(anim_img == NULL) return;
    
    if(frame_index < ANIMATION_TOTAL_FRAMES) {
        current_frame = frame_index;
        lv_img_set_src(anim_img, frames[current_frame]);
    }
}

/* 获取当前帧 */
uint8_t LVGL_Animation_GetCurrentFrame(void)
{
    return current_frame;
}

/* 暂停动画 */
void LVGL_Animation_Pause(void)
{
    if(anim_timer != NULL && !is_paused) {
        lv_timer_pause(anim_timer);
        is_paused = 1;
        printf("LVGL动画已暂停\n");
    }
}

/* 恢复动画 */
void LVGL_Animation_Resume(void)
{
    if(anim_timer != NULL && is_paused) {
        lv_timer_resume(anim_timer);
        is_paused = 0;
        printf("LVGL动画已恢复\n");
    }
}

#include "stm32f10x.h"                  // Device header
#include "SysTick.h"

#include "FreeRTOS.h"
#include "task.h"

#include "bsp_usart.h"
#include "bsp_ili9341_lcd.h"
#include "bsp_bmp.h"
#include "bsp_sdfs_app.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"

#include "animation_frames.h"

#include "ff.h"  // 添加FatFs头文件


/* 任务栈大小 */
#define STARTUP_TASK_STACKSIZE   256
#define LVGL_TASK_STACKSIZE      2048  
#define ANIMATION_TASK_STACKSIZE 512

/* 任务优先级 */
#define STARTUP_TASK_PRIO        1
#define LVGL_TASK_PRIO           2    
#define ANIMATION_TASK_PRIO      1

/* 任务句柄 */
TaskHandle_t StartUpTask_Handle = NULL;
TaskHandle_t LvglTask_Handle = NULL;
TaskHandle_t AnimationTask_Handle = NULL;

/* 全局变量 */
static lv_obj_t *anim_img = NULL;
static uint8_t current_frame = 0;
static lv_timer_t *anim_timer = NULL;

/* 函数声明 */
void StartUpTask(void *pvParameters);
void LvglTask(void *pvParameters);
void AnimationTask(void *pvParameters);
static void animation_timer_cb(lv_timer_t *timer);
void LVGL_Animation_Init(void);


// #define StartUpTask_STACKSIZE 256
// #define StartUpTask_PRIO			1
// TaskHandle_t StartUpTask_Handle;
// void StartUpTask(void * p);

// #define LvglTask_STACKSIZE 512
// #define LvglTask_PRIO			1
// TaskHandle_t LvglTask_Handle;
// void LvglTask(void * p);

// #define FrameSeqTask_STACKSIZE 512
// #define FrameSeqTask_PRIO		2
// TaskHandle_t FrameSeqTask_Handle;
// void FrameSeqTask(void * p);

// extern FrameSequence_t g_frame_seq;


/**
 * @brief  LVGL动画定时器回调
 */
static void animation_timer_cb(lv_timer_t *timer)
{
    if(anim_img == NULL) return;
    
    current_frame++;
    if(current_frame >= TOTAL_FRAMES) {
        current_frame = 0;  /* �·�环播放 */
    }
    
    /* 设置图片源 */
    lv_img_set_src(anim_img, frames[current_frame]);
	// /*淡入淡出效果 */
    // lv_obj_set_style_opa(anim_img, LV_OPA_TRANSP, 0);
    // lv_anim_t a;
    // lv_anim_init(&a);
    // lv_anim_set_var(&a, anim_img);
    // lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    // lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    // lv_anim_set_time(&a, 100);
    // lv_anim_start(&a);
}

/**
 * @brief  初始化LVGL动画
 */
void LVGL_Animation_Init(void)
{
    /* � �建图片控件 */
    anim_img = lv_img_create(lv_scr_act());
    
    /* 设置图片位置*/
    lv_obj_set_pos(anim_img, 0, 0);
    // 居中显示：
	// lv_obj_center(anim_img);
    
    /* 设置图片大小*/
    // lv_obj_set_size(anim_img, 320, 240);
    
    /* 显示第一帧 */
    lv_img_set_src(anim_img, frames[0]);
    
    // /* 创建定时器，每100ms切换一帧（10fps）*/
    // anim_timer = lv_timer_create(animation_timer_cb, 500, NULL);
    
    printf("LVGL动画已启动，总帧数：%d\n", TOTAL_FRAMES);
}



/**
 * @brief  启动任务（调试用）
 */
void StartUpTask(void *pvParameters)
{
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("系统运行中... 当前帧：%d/%d\n", current_frame + 1, TOTAL_FRAMES);
    }
}
/**
 * @brief  LVGL任务
 */
void LvglTask(void *pvParameters)
{
    printf("LVGL任务启动\n");
    
    while(1) {
        lv_timer_handler();  /* 处理LVGL定时器 */
        vTaskDelay(pdMS_TO_TICKS(5));  /* 让出CPU */
    }
}

/**
 * @brief  动画监控任务（可选）
 */
void AnimationTask(void *pvParameters)
{
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


// // 序列帧播放任务
// void FrameSeqTask(void * p){
//     // 初始化序列帧播放器
//     FrameSeq_Init(&g_frame_seq, 1, 5, 1000, 1);  // 从-到-帧，每帧100ms，循环播放
//     FrameSeq_SetPath(&g_frame_seq, "/", ".bmp"); 
    
//     uint16_t x = 0; 
//     uint16_t y = 0; 
    
//     while(1){
//         // 非阻塞方式播放序列帧
//         FrameSeq_Update(&g_frame_seq, x, y);
//         vTaskDelay(pdMS_TO_TICKS(10));  // 让出CPU
//     }
// }

// //阻塞的方式播放
// void PlayAnimationBlocking(void)
// {
//     FrameSequence_t seq;
//     FrameSeq_Init(&seq, 0, 30, 100, 1);  // 0-30帧，每帧100ms，循环
//     FrameSeq_SetPath(&seq, "/", ".bmp");
//     FrameSeq_Play(&seq, 0, 0);  // 阻塞式播放
// }

int main(){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    USART_Config();
    // Sd_fs_init();
    printf("窗口初始化完毕..\n");
    
    // 初始化LVGL
    lv_init();
    lv_port_disp_init();
    //lv_port_indev_init();
    LVGL_Animation_Init();

    // 不显示test按钮
    // LVGL_Test();
    
    printf("屏幕初始化完毕..\n");
    SysTick_Init();
    
    // 创建任务
    // xTaskCreate(StartUpTask, "StartUpTask", StartUpTask_STACKSIZE, NULL, StartUpTask_PRIO, &StartUpTask_Handle);
    // xTaskCreate(LvglTask, "LvglTask", LvglTask_STACKSIZE, NULL, LvglTask_PRIO, &LvglTask_Handle);
    // xTaskCreate(FrameSeqTask, "FrameSeqTask", FrameSeqTask_STACKSIZE, NULL, FrameSeqTask_PRIO, &FrameSeqTask_Handle);
    

	/* 创建FreeRTOS任务 */
    xTaskCreate(StartUpTask, "StartUp", STARTUP_TASK_STACKSIZE, 
                NULL, STARTUP_TASK_PRIO, &StartUpTask_Handle);
    
    xTaskCreate(LvglTask, "LVGL", LVGL_TASK_STACKSIZE, 
                NULL, LVGL_TASK_PRIO, &LvglTask_Handle);
    
    xTaskCreate(AnimationTask, "Animation", ANIMATION_TASK_STACKSIZE, 
                NULL, ANIMATION_TASK_PRIO, &AnimationTask_Handle);

    vTaskStartScheduler();
    
    while(1);  // 不会执行到这里
}
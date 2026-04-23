#include "stm32f10x.h"
#include "bsp_usart1.h"
#include "bsp_key.h"
#include "bsp_lcd.h"
#include "bsp_sdfs_app.h"
#include "test_framework.h"
#include <stdio.h> 

#if (USE_FREERTOS == 1)
    #include "FreeRTOS.h"
    #include "task.h"
#endif

#if (USE_FREERTOS == 0)

static void SysTick_Init(void)
{
    SystemCoreClockUpdate();
    if (SysTick_Config(SystemCoreClock / 1000))
    {
        while(1);
    }
}

#endif

int main(void)
{
    uint8_t key;
    uint32_t last_auto_time = 0;
    uint32_t last_loop_time = 0;
    
    // 基础初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    //USART_Config();
    USARTx_Config();
    Sd_fs_init();
    
#if (USE_FREERTOS == 0)
    SysTick_Init();
#endif

    // 测试框架初始化
    TestFramework_Init(); 
    
    printf("========================================\n");
    printf("STM32测试架启动\n");
    printf("Mode: %s\n", USE_FREERTOS ? "FreeRTOS" : "Bare Metal");
    printf("========================================\n");
    
#if (USE_FREERTOS == 1)
    // ========== FreeRTOS 模式 ==========
    
    // 创建任务并启动调度器
    TestFramework_Run();        // 只创建任务
    vTaskStartScheduler();      // 启动调度器（不会返回）
    while(1);
    
#else
    // ========== 裸机模式 ==========
    RunCurrentTest();  // 运行第一个测试
    
    while(1)
    {
        uint32_t now = GetTickMs();
        
        // 每 20ms 执行一次主循环
        if((now - last_loop_time) >= 20)
        {
            last_loop_time = now;
            
            // 自动轮播：每 3 秒切换一次
            if(TestFramework_GetMode() == TEST_MODE_AUTO)
            {
                if((now - last_auto_time) >= 3000)
                {
                    last_auto_time = now;
                    TestFramework_NextTest();
                }
            }
            
            // 按键检测
            key = Key_Scan();
            switch(key) {
                case KEY0_PRESS:
                    TestFramework_PrevTest();
                    last_auto_time = now;
                    break;
                case KEY1_PRESS:
                    TestFramework_NextTest();
                    last_auto_time = now;
                    break;
                case KEY01_COMBINATION:
                    if(TestFramework_GetMode() == TEST_MODE_AUTO)
                        TestFramework_SetMode(TEST_MODE_MANUAL);
                    else
                        TestFramework_SetMode(TEST_MODE_AUTO);
                    last_auto_time = now;
                    break;
            }
            
            TestFramework_Update();
        }
    }
#endif
}
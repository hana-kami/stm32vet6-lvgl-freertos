#ifndef __TEST_FRAMEWORK_H
#define __TEST_FRAMEWORK_H

#include <stdint.h>
#include "stm32f10x.h"

#if (USE_FREERTOS == 1)
    #include "FreeRTOS.h"
    #include "task.h"
#endif
typedef enum {
    TEST_MODE_MANUAL = 0,
    TEST_MODE_AUTO = 1
} TestMode_t;

// 测试项结构体
typedef struct {
    const char* name;
    void (*init)(void);
    void (*run)(void);
    void (*exit)(void);
} TestItem_t;


#if (USE_FREERTOS == 1)
    // FreeRTOS 方式获取时间
    static uint32_t GetTickMs(void)
    {
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
#else

    extern volatile uint32_t g_sys_tick;

    static uint32_t GetTickMs(void)
    {
        return g_sys_tick;
    }
#endif

// 公共接口
void TestFramework_EnterTest(uint8_t test_index);  // 进入指定测试（只调用一次 init）
void TestFramework_Update(void);                    // 更新当前测试（周期性调用 run）
void TestFramework_Init(void);
void TestFramework_Run(void);
void TestFramework_NextTest(void);
void TestFramework_PrevTest(void);
void TestFramework_SwitchMode(void);

// Getter/Setter
TestMode_t TestFramework_GetMode(void);
void TestFramework_SetMode(TestMode_t mode);
uint8_t TestFramework_GetCurrentTest(void);
const char* TestFramework_GetTestName(uint8_t index);

// 执行当前测试（需要在外部调用）
void RunCurrentTest(void);

#endif /* __TEST_FRAMEWORK_H */
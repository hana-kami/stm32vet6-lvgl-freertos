#include "test_framework.h"
#include "bsp_key.h"
#include "bsp_lcd.h"
#include "bsp_bmp.h"
#include "bmp_browser.h"
#include "bsp_sdfs_app.h"
#include <stdio.h>
#include <string.h>

#if (USE_FREERTOS == 1)
    #include "FreeRTOS.h"
    #include "task.h"
#endif

/* ==================== 测试项函数声明 ==================== */
void Test_SolidColor_Init(void);
void Test_SolidColor_Run(void);
void Test_GrayScale_Init(void);
void Test_GrayScale_Run(void);
void Test_Shapes_Init(void);
void Test_Shapes_Run(void);
void Test_BMPBrowser_Init(void);
void Test_BMPBrowser_Run(void);
void Test_BMPBrowser_Exit(void);
void Test_Text_Init(void);
void Test_Text_Run(void);
void Test_Touch_Init(void);
void Test_Touch_Run(void);

/* ==================== 测试项列表 ==================== */
static const TestItem_t g_test_items[] = {
    {"1. Red Screen",     Test_SolidColor_Init, Test_SolidColor_Run, NULL},
    {"2. Green Screen",   Test_SolidColor_Init, Test_SolidColor_Run, NULL},
    {"3. Blue Screen",    Test_SolidColor_Init, Test_SolidColor_Run, NULL},
    {"4. White Screen",   Test_SolidColor_Init, Test_SolidColor_Run, NULL},
    {"5. Black Screen",   Test_SolidColor_Init, Test_SolidColor_Run, NULL},
    {"6. Gray Scale",     Test_GrayScale_Init,  Test_GrayScale_Run,  NULL},
    {"7. Shapes Test",    Test_Shapes_Init,     Test_Shapes_Run,     NULL},
    {"8. Text Display",   Test_Text_Init,       Test_Text_Run,       NULL},
    {"9. BMP Browser",    Test_BMPBrowser_Init, Test_BMPBrowser_Run, Test_BMPBrowser_Exit},
};

#define TEST_COUNT (sizeof(g_test_items) / sizeof(TestItem_t))

/* ==================== 静态变量 ==================== */
static TestMode_t g_current_mode = TEST_MODE_MANUAL;
static uint8_t g_current_test = 0;
static uint8_t g_test_initialized = 0;  // 标记当前测试是否已初始化

/* 颜色列表 */
static const uint16_t g_colors[] = {
    LCD_COLOR_RED,
    LCD_COLOR_GREEN, 
    LCD_COLOR_BLUE,
    LCD_COLOR_WHITE,
    LCD_COLOR_BLACK
};
#define COLOR_COUNT (sizeof(g_colors) / sizeof(uint16_t))

#if (USE_FREERTOS == 1)
    static TaskHandle_t g_key_task_handle = NULL;
    static TaskHandle_t g_auto_task_handle = NULL;
#endif

/* ==================== 辅助函数 ==================== */

// 非阻塞延时（使用状态机）
typedef struct {
    uint32_t start_time;
    uint32_t delay_ms;
    uint8_t active;
} DelayTimer_t;

static DelayTimer_t g_delay_timer = {0, 0, 0};

#if (USE_FREERTOS == 1)
    // FreeRTOS 方式获取时间
    static uint32_t GetTickMs(void)
    {
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
#else

    // extern volatile uint32_t g_sys_tick;
    
    static uint32_t GetTickMs(void)
    {
        return 0;
    }
#endif

// 启动延时
static void StartDelay(uint32_t ms)
{
    g_delay_timer.start_time = GetTickMs();
    g_delay_timer.delay_ms = ms;
    g_delay_timer.active = 1;
}

// 检查延时是否完成
static uint8_t IsDelayComplete(void)
{
    if(!g_delay_timer.active) return 1;
    
    if((GetTickMs() - g_delay_timer.start_time) >= g_delay_timer.delay_ms)
    {
        g_delay_timer.active = 0;
        return 1;
    }
    return 0;
}

// 显示标题栏
static void ShowTitleBar(const char* title)
{
    LCD_Clear(LCD_COLOR_BLACK);
    LCD_DrawRectangle(0, 0, LCD_GetWidth(), 30, LCD_COLOR_BLUE, 1);
    LCD_ShowString(10, 8, title, LCD_COLOR_BLUE, LCD_COLOR_WHITE);
    
    if(g_current_mode == TEST_MODE_AUTO) {
        LCD_ShowString(170, 8, "[AUTO]", LCD_COLOR_BLUE, LCD_COLOR_YELLOW);
    } else {
        LCD_ShowString(170, 8, "[MANUAL]", LCD_COLOR_BLUE, LCD_COLOR_GREEN);
    }
    
    LCD_ShowString(10, LCD_GetHeight() - 20, "K1:PREV K12:MODE K2:NEXT", 
                   LCD_COLOR_BLACK, LCD_COLOR_WHITE);
}

/* ==================== 测试项实现 ==================== */

// 纯色屏幕测试状态机
typedef enum {
    SOLID_STATE_SHOW_COLOR = 0,
    SOLID_STATE_WAIT,
    SOLID_STATE_SHOW_TITLE,
    SOLID_STATE_DONE,      // 新增：完成状态
} SolidColorState_t;

static SolidColorState_t solid_state = SOLID_STATE_SHOW_COLOR;
static uint32_t solid_start_time = 0;

void Test_SolidColor_Init(void)
{
    solid_state = SOLID_STATE_SHOW_COLOR;  // 重置状态机
    solid_start_time = 0;
}

void Test_SolidColor_Run(void)
{
    const char* color_name;
    uint8_t color_index = g_current_test;
    
    if(color_index >= COLOR_COUNT) return;
    
    switch(solid_state)
    {
        case SOLID_STATE_SHOW_COLOR:
            LCD_Clear(g_colors[color_index]);
            solid_state = SOLID_STATE_WAIT;
            solid_start_time = GetTickMs();
            break;
            
        case SOLID_STATE_WAIT:
            if((GetTickMs() - solid_start_time) >= 2000)
            {
                solid_state = SOLID_STATE_SHOW_TITLE;
            }
            break;
            
        case SOLID_STATE_SHOW_TITLE:
            color_name = "";
            switch(color_index) {
                case 0: color_name = "RED SCREEN"; break;
                case 1: color_name = "GREEN SCREEN"; break;
                case 2: color_name = "BLUE SCREEN"; break;
                case 3: color_name = "WHITE SCREEN"; break;
                case 4: color_name = "BLACK SCREEN"; break;
            }
            ShowTitleBar(color_name);
            solid_state = SOLID_STATE_DONE;  // 进入完成状态，不再变化
            break;
            
        case SOLID_STATE_DONE:
            break;
    }
}
typedef enum {
    GRAY_STATE_SHOW = 0,
    GRAY_STATE_WAIT,
    GRAY_STATE_SHOW_TITLE,
    GRAY_STATE_DONE,
} GrayState_t;

static GrayState_t gray_state = GRAY_STATE_SHOW;
static uint32_t gray_start_time = 0;

void Test_GrayScale_Init(void)
{
    gray_state = GRAY_STATE_SHOW;
    gray_start_time = 0;
}

void Test_GrayScale_Run(void)
{
    switch(gray_state)
    {
        case GRAY_STATE_SHOW:
            disp_gray();
            gray_state = GRAY_STATE_WAIT;
            gray_start_time = GetTickMs();
            break;
            
        case GRAY_STATE_WAIT:
            if((GetTickMs() - gray_start_time) >= 2000)
            {
                gray_state = GRAY_STATE_SHOW_TITLE;
            }
            break;
            
        case GRAY_STATE_SHOW_TITLE:
            ShowTitleBar("GRAY SCALE TEST");
            gray_state = GRAY_STATE_DONE;
            break;
            
        case GRAY_STATE_DONE:
            break;
    }
}

typedef enum {
    SHAPES_STATE_SHOW = 0,
    SHAPES_STATE_WAIT,
    SHAPES_STATE_SHOW_TITLE,
    SHAPES_STATE_DONE,
} ShapesState_t;

static ShapesState_t shapes_state = SHAPES_STATE_SHOW;
static uint32_t shapes_start_time = 0;

void Test_Shapes_Init(void)
{
    shapes_state = SHAPES_STATE_SHOW;
    shapes_start_time = 0;
}

void Test_Shapes_Run(void)
{
    uint16_t width;
    uint16_t height;
    uint16_t center_x;
    uint16_t center_y;
    switch(shapes_state)
    {
        case SHAPES_STATE_SHOW:
            ShowTitleBar("SHAPES TEST");
    
            width = LCD_GetWidth();
            height = LCD_GetHeight();
            center_x = width / 2;
            center_y = height / 2;
            
            // 画矩形
            LCD_DrawRectangle(20, 40, 80, 60, LCD_COLOR_RED, 0);
            LCD_DrawRectangle(110, 40, 80, 60, LCD_COLOR_GREEN, 1);
            
            // 画圆
            LCD_DrawCircle(center_x - 60, center_y + 40, 30, LCD_COLOR_BLUE, 0);
            LCD_DrawCircle(center_x + 60, center_y + 40, 30, LCD_COLOR_YELLOW, 1);
            
            // 画线
            LCD_DrawLine(20, height - 40, width - 20, height - 40, LCD_COLOR_CYAN);
            LCD_DrawLine(width / 2, 80, width / 2, height - 60, LCD_COLOR_MAGENTA);
            
            // 画对角线
            LCD_DrawLine(0, 0, width - 1, height - 1, LCD_COLOR_WHITE);
            LCD_DrawLine(0, height - 1, width - 1, 0, LCD_COLOR_WHITE);
            shapes_state = SHAPES_STATE_WAIT;
            shapes_start_time = GetTickMs();
            break;
            
        case SHAPES_STATE_WAIT:
            if((GetTickMs() - shapes_start_time) >= 2000)
            {
                shapes_state = SHAPES_STATE_SHOW_TITLE;
            }
            break;

        case SHAPES_STATE_SHOW_TITLE:
            ShowTitleBar("SHAPES TEST");
            shapes_state = SHAPES_STATE_DONE;
            break;
            
        case SHAPES_STATE_DONE:
            break;
    }
}


typedef enum {
    TEXT_STATE_SHOW = 0,
    TEXT_STATE_WAIT,
    TEXT_STATE_SHOW_TITLE,
    TEXT_STATE_DONE,
} TextState_t;

static TextState_t text_state = TEXT_STATE_SHOW;
static uint32_t text_start_time = 0;

void Test_Text_Init(void)
{
    text_state = TEXT_STATE_SHOW;
    text_start_time = 0;
}

void Test_Text_Run(void)
{
    uint16_t y;
    uint16_t line_height;
    char hex_buf[20];
    switch(text_state)
    {
        case TEXT_STATE_SHOW:
            ShowTitleBar("TEXT DISPLAY TEST");
    
            y = 50;
            line_height = 20;
            
            LCD_ShowString(10, y, "Hello STM32!", LCD_COLOR_BLACK, LCD_COLOR_GREEN);
            y += line_height;
            
            LCD_ShowString(10, y, "LCD Test Framework", LCD_COLOR_BLACK, LCD_COLOR_YELLOW);
            y += line_height;
            
            LCD_ShowString(10, y, "Number: ", LCD_COLOR_BLACK, LCD_COLOR_CYAN);
            LCD_ShowNumber(80, y, 12345, 5, LCD_COLOR_BLACK, LCD_COLOR_CYAN);
            y += line_height;
            
            LCD_ShowString(10, y, "Hex: 0x", LCD_COLOR_BLACK, LCD_COLOR_MAGENTA);
            // 显示十六进制
            sprintf(hex_buf, "%04X", 0xABCD);
            LCD_ShowString(70, y, hex_buf, LCD_COLOR_BLACK, LCD_COLOR_MAGENTA);
            text_state = TEXT_STATE_WAIT;
            text_start_time = GetTickMs();
            break;
            
        case TEXT_STATE_WAIT:
            if((GetTickMs() - text_start_time) >= 2000)
            {
                text_state = TEXT_STATE_SHOW_TITLE;
            }
            break;
            
        case TEXT_STATE_SHOW_TITLE:
            ShowTitleBar("TEXT DISPLAY TEST");
            text_state = TEXT_STATE_DONE;
            break;
            
        case TEXT_STATE_DONE:
            break;
    }
}

// BMP浏览器测试状态机
typedef enum {
    BMP_STATE_SHOW = 0,
    BMP_STATE_WAIT,
    BMP_STATE_SHOW_TITLE,
    BMP_STATE_DONE,
} BMPState_t;

static BMPState_t bmp_state = BMP_STATE_SHOW;
static uint32_t bmp_start_time = 0;

void Test_BMPBrowser_Init(void)
{
    bmp_state = BMP_STATE_SHOW;
    bmp_start_time = 0;
}

void Test_BMPBrowser_Run(void)
{
    switch(bmp_state)
    {
        case BMP_STATE_SHOW:
            Lcd_Show_bmp(0, 0, "/1.bmp");
            bmp_state = BMP_STATE_WAIT;
            bmp_start_time = GetTickMs();
            break;
            
        case BMP_STATE_WAIT:
            if((GetTickMs() - bmp_start_time) >= 2000)
            {
                bmp_state = BMP_STATE_SHOW_TITLE;
            }
            break;
            
        case BMP_STATE_SHOW_TITLE:
            // 显示标题栏
            ShowTitleBar("BMP BROWSER");
            bmp_state = BMP_STATE_DONE;
            break;
            
        case BMP_STATE_DONE:
            // 保持显示，什么都不做
            break;
    }
}

void Test_BMPBrowser_Exit(void)
{
    // 不需要处理
}
/* ==================== 框架核心函数 ==================== */

void TestFramework_EnterTest(uint8_t test_index)
{
    if(g_test_items[g_current_test].exit) {
        g_test_items[g_current_test].exit();
    }
    
    g_current_test = test_index;
    
    if(g_test_items[g_current_test].init) {
        g_test_items[g_current_test].init();
    }

    g_test_initialized = 0;
    
    printf("进入测试: %s\n", g_test_items[g_current_test].name);
}

// 更新当前测试（主循环中周期性调用）
void TestFramework_Update(void)
{
    if(g_test_items[g_current_test].run) {
        g_test_items[g_current_test].run();
    }
}

void RunCurrentTest(void)
{
    TestFramework_EnterTest(g_current_test);
}

#if (USE_FREERTOS == 1)
// FreeRTOS 按键任务
static void KeyTask(void *pvParameters)
{
    uint8_t key;
    while(1) {
        key = Key_Scan();
        if(key != KEY_NONE) {
            if(g_current_test == 8) {
                switch(key) {
                    case KEY0_PRESS: BMP_Browser_Prev(); break;
                    case KEY2_PRESS: BMP_Browser_Next(); break;
                    case KEY1_PRESS: TestFramework_SwitchMode(); break;
                }
            } else {
                switch(key) {
                    case KEY0_PRESS: TestFramework_PrevTest(); break;
                    case KEY1_PRESS: TestFramework_SwitchMode(); break;
                    case KEY2_PRESS: TestFramework_NextTest(); break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// FreeRTOS 自动轮播任务
static void AutoTask(void *pvParameters)
{
    uint8_t auto_counter = 0;
    while(1) {
        if(g_current_mode == TEST_MODE_AUTO) {
            auto_counter++;
            if(auto_counter >= 30) {
                auto_counter = 0;
                TestFramework_NextTest();
            }
        } else {
            auto_counter = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
#endif

/* ==================== 公共API ==================== */

void TestFramework_Init(void)
{
    Key_GPIO_Config();
    LCD_Init();
    LCD_BacklightControl(1);
    LCD_Clear(LCD_COLOR_BLACK);
    
    printf("========================================\n");
    printf("测试框架初始化完成\n");
    printf("共%d个测试项\n", TEST_COUNT);
    printf("========================================\n");
}

void TestFramework_Run(void)
{
#if (USE_FREERTOS == 1)
    xTaskCreate(KeyTask, "KeyTask", 256, NULL, 2, &g_key_task_handle);
    xTaskCreate(AutoTask, "AutoTask", 256, NULL, 1, &g_auto_task_handle);
#endif
    RunCurrentTest();
}

void TestFramework_NextTest(void)
{
    uint8_t new_test = g_current_test + 1;
    if(new_test >= TEST_COUNT) {
        new_test = 0;
    }
    TestFramework_EnterTest(new_test);
}

void TestFramework_PrevTest(void)
{
    uint8_t new_test;
    if(g_current_test == 0) {
        new_test = TEST_COUNT - 1;
    } else {
        new_test = g_current_test - 1;
    }
    TestFramework_EnterTest(new_test);
}

void TestFramework_SwitchMode(void)
{
    if(g_current_mode == TEST_MODE_MANUAL) {
        g_current_mode = TEST_MODE_AUTO;
        printf("切换到自动模式\n");
        LCD_ShowString(LCD_GetWidth() / 2 - 30, LCD_GetHeight() / 2, 
                       "AUTO MODE", LCD_COLOR_BLACK, LCD_COLOR_YELLOW);
    } else {
        g_current_mode = TEST_MODE_MANUAL;
        printf("切换到手动模式\n");
        LCD_ShowString(LCD_GetWidth() / 2 - 35, LCD_GetHeight() / 2, 
                       "MANUAL MODE", LCD_COLOR_BLACK, LCD_COLOR_GREEN);
    }
    
#if (USE_FREERTOS == 1)
    vTaskDelay(pdMS_TO_TICKS(500));
#endif
    if(g_test_items[g_current_test].run) {
        g_test_items[g_current_test].run();
    }
}

TestMode_t TestFramework_GetMode(void)
{
    return g_current_mode;
}

void TestFramework_SetMode(TestMode_t mode)
{
    g_current_mode = mode;
    printf("模式设置为: %s\n", mode == TEST_MODE_AUTO ? "自动" : "手动");
    
    if(mode == TEST_MODE_AUTO) {
        LCD_ShowString(LCD_GetWidth() / 2 - 30, LCD_GetHeight() / 2, 
                       "AUTO MODE", LCD_COLOR_BLACK, LCD_COLOR_YELLOW);
        LCD_ShowString(170, 8, "[AUTO]", LCD_COLOR_BLUE, LCD_COLOR_YELLOW);
    } else {
        LCD_ShowString(LCD_GetWidth() / 2 - 35, LCD_GetHeight() / 2, 
                       "MANUAL MODE", LCD_COLOR_BLACK, LCD_COLOR_GREEN);
        LCD_ShowString(170, 8, "[MANUAL]", LCD_COLOR_BLUE, LCD_COLOR_GREEN);
    }
}

uint8_t TestFramework_GetCurrentTest(void)
{
    return g_current_test;
}

const char* TestFramework_GetTestName(uint8_t index)
{
    if(index < TEST_COUNT) {
        return g_test_items[index].name;
    }
    return NULL;
}
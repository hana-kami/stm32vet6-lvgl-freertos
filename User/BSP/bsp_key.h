#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "stm32f10x.h"

// 按键引脚定义
#define macKEY1_GPIO_CLK     RCC_APB2Periph_GPIOC
#define macKEY1_GPIO_PORT    GPIOC
#define macKEY1_GPIO_PIN     GPIO_Pin_0

#define macKEY2_GPIO_CLK     RCC_APB2Periph_GPIOC
#define macKEY2_GPIO_PORT    GPIOC
#define macKEY2_GPIO_PIN     GPIO_Pin_1

#define macKEY3_GPIO_CLK     RCC_APB2Periph_GPIOC
#define macKEY3_GPIO_PORT    GPIOC
#define macKEY3_GPIO_PIN     GPIO_Pin_2


// 按键值定义
#define KEY_ON                1
#define KEY_OFF               0

#define KEY_NONE              0
#define KEY0_PRESS            1
#define KEY1_PRESS            2
#define KEY2_PRESS            3
#define KEY01_COMBINATION     4

// 0: 使用非阻塞模式（需要周期性调用 Key_Scan）
// 1: 使用阻塞模式（等待按键释放）
#define KEY_SCAN_BLOCKING_MODE    0 

// 函数声明
void Key_GPIO_Config(void);

#if (KEY_SCAN_BLOCKING_MODE == 1)
// 阻塞模式：等待按键释放
uint8_t Key_Scan(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
#else

// 非阻塞模式：立即返回，需要周期性调用
uint8_t Key_Scan(void);
#endif

#endif /* __BSP_KEY_H */
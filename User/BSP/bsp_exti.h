#ifndef __EXTI_H
#define	__EXTI_H


#include "stm32f10x.h"


/*外部中断EXIT相关宏定义*/
//#define             macEXTI_GPIO_CLK                        (RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO)     
//#define             macEXTI_GPIO_PORT                       GPIOE   
//#define             macEXTI_GPIO_PIN                        GPIO_Pin_4
//#define             macEXTI_SOURCE_PORT                     GPIO_PortSourceGPIOE
//#define             macEXTI_SOURCE_PIN                      GPIO_PinSource4
//#define             macEXTI_LINE                            EXTI_Line4
//#define             macEXTI_IRQ                             EXTI4_IRQn
//#define             macEXTI_INT_FUNCTION                    EXTI4_IRQHandler


#define             macEXTI_GPIO_CLK                        (RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO)     
#define             macEXTI_GPIO_PORT                       GPIOE   
#define             macEXTI_GPIO_PIN                        GPIO_Pin_4
#define             macEXTI_SOURCE_PORT                     GPIO_PortSourceGPIOE
#define             macEXTI_SOURCE_PIN                      GPIO_PinSource4
#define             macEXTI_LINE                            EXTI_Line4
#define             macEXTI_IRQ                             EXTI4_IRQn
#define             macEXTI_INT_FUNCTION                    EXTI4_IRQHandler


//void                TSC2046_EXTI_Config                     ( void );


#endif /* __EXTI_H */


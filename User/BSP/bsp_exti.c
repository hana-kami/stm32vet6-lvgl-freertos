/**
  ******************************************************************************
  * @file    bsp_exti.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   外部中断应用 BSP
  ******************************************************************************
  * @attention
  *
  * 实验平台: 野火 ISO-MINI STM32 开发板
  * 论坛    : http://www.chuxue123.com
  * 淘宝    : http://firestm32.taobao.com
  *
  ******************************************************************************
  */
	
#include "bsp_exti.h"

static void NVIC_Configuration(void);

/**
  * @brief  配置外部中断的 NVIC（嵌套向量中断控制器）
  * @note   重要: 不要在这里设置优先级分组！
  *         优先级分组应该在 main 函数开始时统一设置
  * @param  无
  * @retval 无
  */
static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* ⚠️ 删除这行！不要在这里设置优先级分组 */
    /* NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);  // ← 必须删除或注释掉 */
    
    /* 配置外部中断优先级 */
    /* 注意: FreeRTOS 要求中断优先级必须 >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5)
       所以这里设置为 6 或更高（数值越大优先级越低）*/
    NVIC_InitStructure.NVIC_IRQChannel = macEXTI_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;  /* 改为 6，不要用 0 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         /* 分组4 时子优先级无效 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  配置 PE4 为外部中断口，配置中断优先级
  * @param  无
  * @retval 无
  */
void EXTI_Pxy_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; 
    EXTI_InitTypeDef EXTI_InitStructure;
    
    /* 使能外部中断线时钟和 AFIO 时钟 */
    RCC_APB2PeriphClockCmd(macEXTI_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  /* 使能 AFIO 时钟 */
    
    /* 配置 NVIC */
    NVIC_Configuration();
    
    /* EXTI 线 GPIO 配置 */
    GPIO_InitStructure.GPIO_Pin = macEXTI_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  /* 上拉输入 */
    GPIO_Init(macEXTI_GPIO_PORT, &GPIO_InitStructure);
    
    /* EXTI 线模式配置 */
    GPIO_EXTILineConfig(macEXTI_SOURCE_PORT, macEXTI_SOURCE_PIN);
    EXTI_InitStructure.EXTI_Line = macEXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  /* 下降沿触发中断 */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

/* 备选配置（触摸屏中断）- 已注释，如需使用请取消注释并按相同方式修改 */
/*
static void TP_INT_NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // ⚠️ 删除这行！不要在这里设置优先级分组
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    
    // 使用 EXTI4 中断
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;  // 使用 6
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void EXTI_Pxy_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; 
    EXTI_InitTypeDef EXTI_InitStructure;
    
    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
    
    // 配置 NVIC
    TP_INT_NVIC_Config();
    
    // EXTI 线 GPIO 配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // 下拉输入
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    // EXTI 线模式配置
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);  // PE4
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  // 下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}
*/

/*********************************************END OF FILE**********************/
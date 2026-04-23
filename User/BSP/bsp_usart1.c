/**
  ******************************************************************************
  * @file    bsp_usart1.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   USART1 应用 BSP
  ******************************************************************************
  * @attention
  *
  * 实验平台: 野火 iSO-MINI STM32 开发板
  * 论坛    : http://www.chuxue123.com
  * 淘宝    : http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_usart1.h"

/**
  * @brief  USARTx GPIO 配置, 工作模式配置, 115200 8-N-1
  * @param  无
  * @retval 无
  */
void USARTx_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    /* 使能 USART1 时钟 */
    macUSART_APBxClock_FUN(macUSART_CLK, ENABLE);
    macUSART_GPIO_APBxClock_FUN(macUSART_GPIO_CLK, ENABLE);
    
    /* USART1 GPIO 配置 */
    /* 配置 USART1 Tx (PA.09) 为复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin =  macUSART_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(macUSART_TX_PORT, &GPIO_InitStructure);    
    
    /* 配置 USART1 Rx (PA.10) 为浮空输入 */
    GPIO_InitStructure.GPIO_Pin = macUSART_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(macUSART_RX_PORT, &GPIO_InitStructure);
    
    /* USART1 工作模式配置 */
    USART_InitStructure.USART_BaudRate = macUSART_BAUD_RATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(macUSARTx, &USART_InitStructure);
    
    /* 使能串口1接收中断 */
    USART_ITConfig(macUSARTx, USART_IT_RXNE, ENABLE);
    
    USART_Cmd(macUSARTx, ENABLE);
}

/**
  * @brief  配置 USART1 的中断优先级
  * @note   注意: 不要在多个地方调用 NVIC_PriorityGroupConfig!
  *         优先级分组应该在 main 函数开始时统一设置一次
  * @param  无
  * @retval 无
  */
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* ⚠️ 重要: 删除这行！优先级分组应该在 main 函数中统一设置 */
    /* NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  // ← 必须删除或注释掉 */
    
    /* 配置 USART1 中断优先级 */
    /* 注意: FreeRTOS 要求中断优先级必须 >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5)
       所以这里设置为 6 或更高（数值越大优先级越低）*/
    NVIC_InitStructure.NVIC_IRQChannel = macUSART_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;  /* 改为 6，不要用 0 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         /* 分组4 时子优先级无效 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  重定向 printf 函数到 USART1
  * @param  ch: 要发送的字符
  * @param  f: 文件指针
  * @retval 发送的字符
  */
int fputc(int ch, FILE *f)
{
    /* 发送一个字节数据到 USART1 */
    USART_SendData(macUSARTx, (uint8_t) ch);
    
    /* 等待发送完成 */
    while (USART_GetFlagStatus(macUSARTx, USART_FLAG_TXE) == RESET);
    
    return (ch);
}

/**
  * @brief  重定向 scanf 函数到 USART1
  * @param  f: 文件指针
  * @retval 接收的字符
  */
int fgetc(FILE *f)
{
    /* 等待接收到数据 */
    while (USART_GetFlagStatus(macUSARTx, USART_FLAG_RXNE) == RESET);
    
    return (int)USART_ReceiveData(macUSARTx);
}

/*********************************************END OF FILE**********************/
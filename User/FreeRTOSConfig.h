/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

*/


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#ifndef USE_FREERTOS
    #define USE_FREERTOS 0
#endif


#include <stdio.h>


// 针对不同编译器，包含不同的stdint.h文件
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
    #include <stdint.h>
    extern uint32_t SystemCoreClock;
#endif


// 断言宏定义（调试用）
#define vAssertCalled(char,int) printf("Error:%s,%d\r\n",char,int)
#define configASSERT(x) if((x)==0) vAssertCalled(__FILE__,__LINE__)


/************************************************************************
 *              FreeRTOS基础配置选项 
 *********************************************************************/

/* 1:RTOS使用抢占式调度器，0:RTOS使用协作式调度器（时间片轮转）*/
#define configUSE_PREEMPTION                      1

/* 1:使用时间片轮转调度（默认使能）*/
#define configUSE_TIME_SLICING                    1

/* STM32F103不支持硬件CLZ指令，必须设置为0 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    0                       
                                                                        
/* 1:使能低功耗tickless模式，0:系统tick中断一直运行 */
#define configUSE_TICKLESS_IDLE                   0   

/*
 * CPU主频（Hz），即CPU指令执行频率
 * 系统时钟频率，通常为72MHz（STM32F103常用值）
 */
#define configCPU_CLOCK_HZ                        (SystemCoreClock)

/* RTOS系统节拍中断频率，单位Hz。1000表示1ms一次中断 */
#define configTICK_RATE_HZ                        ((TickType_t)1000)

/* 任务最大优先级数量（建议5-10，过大会浪费RAM） */
#define configMAX_PRIORITIES                      (5)

/* 空闲任务使用的栈大小（单位：字，1字=4字节） */
#define configMINIMAL_STACK_SIZE                  ((unsigned short)128)
  
/* 任务名称字符串最大长度（不含结束符'\0'） */
#define configMAX_TASK_NAME_LEN                   (16)

/* 系统节拍计数器位数：0表示32位无符号整数，1表示16位无符号整数 */
#define configUSE_16_BIT_TICKS                    0                      

/* 空闲任务是否让出CPU使用权（相同优先级时） */
#define configIDLE_SHOULD_YIELD                   1           

/* 队列集功能（用不到可以设为0）*/
#define configUSE_QUEUE_SETS                      0    

/* 任务通知功能（默认开启）*/
#define configUSE_TASK_NOTIFICATIONS              1   

/* 互斥信号量（用不到可以设为0）*/
#define configUSE_MUTEXES                         0    

/* 递归互斥信号量（用不到可以设为0）*/                                            
#define configUSE_RECURSIVE_MUTEXES               0   

/* 计数信号量（用不到可以设为0）*/
#define configUSE_COUNTING_SEMAPHORES             0

/* 队列注册表大小（用于调试，用不到可以设为0）*/
#define configQUEUE_REGISTRY_SIZE                 10                                
                                                                       
#define configUSE_APPLICATION_TASK_TAG            0                       
                      

/*****************************************************************
 *          FreeRTOS内存管理相关配置                                               
*****************************************************************/
/* 支持动态内存分配 */
#define configSUPPORT_DYNAMIC_ALLOCATION           1    
/* 支持静态内存分配（暂时不需要，设为0）*/
#define configSUPPORT_STATIC_ALLOCATION            0					
/* 系统总堆大小（单位：字节）STM32F103根据实际RAM调整 */
#define configTOTAL_HEAP_SIZE                      ((size_t)(25  * 1024))    


/***************************************************************
 *          FreeRTOS钩子函数相关配置                                            
**************************************************************/
/* 1:使能空闲任务钩子函数（Idle Hook），0:禁用 */
#define configUSE_IDLE_HOOK                        0      

/* 1:使能时间片钩子函数（Tick Hook），0:禁用 */
#define configUSE_TICK_HOOK                        0           

/* 1:使能内存分配失败钩子函数 */
#define configUSE_MALLOC_FAILED_HOOK               0 

/*
 * 栈溢出检测功能
 * 0:禁用
 * 1:使用方法1（较简单，在任务切换时检测）
 * 2:使用方法2（较可靠，在任务切换时检测并填充特定值）
 */
#define configCHECK_FOR_STACK_OVERFLOW             0   


/********************************************************************
 *         FreeRTOS运行时和状态统计相关配置   
 **********************************************************************/
/* 运行时统计功能（用不到设为0）*/
#define configGENERATE_RUN_TIME_STATS              0             
/* 可视化追踪功能（用不到设为0）*/
#define configUSE_TRACE_FACILITY                   0    
/* 格式化统计函数（需要configUSE_TRACE_FACILITY为1时才有用）*/
#define configUSE_STATS_FORMATTING_FUNCTIONS       1                       
                                                                        
                                                                        
/********************************************************************
 *           FreeRTOS协程相关配置                                                
*********************************************************************/
/* 协程功能（用不到设为0）*/
#define configUSE_CO_ROUTINES                      0                 
/* 协程有效优先级数目 */
#define configMAX_CO_ROUTINE_PRIORITIES            (2)                   


/***********************************************************************
 *           FreeRTOS软件定时器相关配置      
 **********************************************************************/
/* 软件定时器功能（用不到设为0）*/
#define configUSE_TIMERS                           0                              
/* 软件定时器任务优先级 */
#define configTIMER_TASK_PRIORITY                  (configMAX_PRIORITIES - 1)        
/* 软件定时器命令队列长度 */
#define configTIMER_QUEUE_LENGTH                   10                               
/* 软件定时器任务栈大小（单位：字）*/
#define configTIMER_TASK_STACK_DEPTH               (configMINIMAL_STACK_SIZE * 2)    

/************************************************************
 *           FreeRTOS可选API函数配置                                                     
 ************************************************************/
#define INCLUDE_xTaskGetSchedulerState             1                       
#define INCLUDE_vTaskPrioritySet                   1
#define INCLUDE_uxTaskPriorityGet                  1
#define INCLUDE_vTaskDelete                        1
#define INCLUDE_vTaskCleanUpResources              1
#define INCLUDE_vTaskSuspend                       1
#define INCLUDE_vTaskDelayUntil                    1
#define INCLUDE_vTaskDelay                         1
#define INCLUDE_eTaskGetState                      1
#define INCLUDE_xTimerPendFunctionCall             0
#define INCLUDE_xTaskGetCurrentTaskHandle          1
#define INCLUDE_uxTaskGetStackHighWaterMark        1    // 使能栈高水位检测
#define INCLUDE_xTaskGetIdleTaskHandle             0


/******************************************************************
 *           FreeRTOS中断相关配置                                                 
 ******************************************************************/
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS                         __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS                         4                   // STM32F103使用4位优先级
#endif

/* 最低中断优先级（数值越大优先级越低）*/
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY     15     

/* 系统可安全调用的最高中断优先级（数值越小优先级越高）*/
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5 

/* 内核中断优先级（用于SysTick和PendSV）*/
#define configKERNEL_INTERRUPT_PRIORITY             (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* 可调用FreeRTOS API的最高中断优先级 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY        (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))


/****************************************************************
 *           FreeRTOS中断服务函数映射（重要！）                         
 ****************************************************************/
/* 将FreeRTOS的函数名映射到启动文件中的标准中断名 */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler


/* 如果启用追踪功能，则包含Tracealyzer头文件 */
#if (configUSE_TRACE_FACILITY == 1)
    #include "trcRecorder.h"
#endif

#endif /* FREERTOS_CONFIG_H */
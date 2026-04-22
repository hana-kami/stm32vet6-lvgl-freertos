#include "bsp_key.h"
#include <stdio.h>

// 保存上一次按键状态
static uint8_t oldStat = 0;

// 简单延时（用于消抖）
static void Key_Delay(uint32_t nCount)
{
    for(; nCount != 0; nCount--);
}

void Key_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能时钟（使用你原来的宏）
    RCC_APB2PeriphClockCmd(macKEY1_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(macKEY2_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(macKEY3_GPIO_CLK, ENABLE);
    
    // 配置为上拉输入（因为按下是低电平）
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    
    // 注意：GPIO_Init 的第一个参数是 GPIO_TypeDef*，不是时钟宏
    GPIO_InitStructure.GPIO_Pin = macKEY1_GPIO_PIN;
    GPIO_Init(macKEY1_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = macKEY2_GPIO_PIN;
    GPIO_Init(macKEY2_GPIO_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = macKEY3_GPIO_PIN;
    GPIO_Init(macKEY3_GPIO_PORT, &GPIO_InitStructure);
}

// 非阻塞按键扫描（使用你验证过的逻辑）
uint8_t Key_Scan(void)
{
    uint8_t btnStat;
    uint8_t key_value = KEY_NONE;
    
    // 读取按键状态
    btnStat = (GPIO_ReadInputDataBit(macKEY1_GPIO_PORT, macKEY1_GPIO_PIN)) |
              (GPIO_ReadInputDataBit(macKEY2_GPIO_PORT, macKEY2_GPIO_PIN)) << 1 |
              (GPIO_ReadInputDataBit(macKEY3_GPIO_PORT, macKEY3_GPIO_PIN)) << 2;
    
    // 取反：按下变为1，未按下为0
    btnStat = (~btnStat) & 0x07;
    
    if(btnStat)
    {
        // ========== 组合键检测：KEY1 + KEY2 同时按下 ==========
        // 检查是否同时按下了 KEY1(bit1) 和 KEY2(bit2)
        if(((btnStat & 0x03) == 0x03) && ((oldStat & 0x03) != 0x03))
        {
            key_value = KEY01_COMBINATION;  // 新增的组合键返回值
            // printf("KEY1 + KEY2 同时按下\n");
        }
        // ========== 单个按键检测 ==========
        // KEY1 (bit1) - 单独按下
        else if(((btnStat & 0x02) == 0x02) && ((oldStat & 0x02) == 0x00))
        {
            key_value = KEY1_PRESS;
            // printf("KEY1 按下\n");
        }
        // KEY2 (bit2) - 单独按下
        else if(((btnStat & 0x04) == 0x04) && ((oldStat & 0x04) == 0x00))
        {
            key_value = KEY2_PRESS;
            // printf("KEY2 按下\n");
        }
        // KEY0 (bit0) - 单独按下
        else if(((btnStat & 0x01) == 0x01) && ((oldStat & 0x01) == 0x00))
        {
            key_value = KEY0_PRESS;
            // printf("KEY0 按下\n");
        }
        
        // 消抖延时
        Key_Delay(0xAFFF);
        Key_Delay(0xAFFF);
        Key_Delay(0xAFFF);
    }
    
    // 保存当前状态用于下次边沿检测
    oldStat = btnStat;
    
    return key_value;
}
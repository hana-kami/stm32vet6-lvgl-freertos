/**
 * @file bsp_st75161_lcd.c
 * @brief ST75161 LCD屏幕驱动（并行8080接口）
 * @note 适用于ST75161驱动IC的单色/灰度LCD屏幕
 */

#include "bsp_st75161_lcd.h"
#include "bsp_lcd_config.h"
//#include "pic1.h"	
#if (CURRENT_LCD_TYPE == LCD_TYPE_ST75161)

#include "ascii.h"	

/* ==================== 全局变量 ==================== */
unsigned int Set_Contrast = 322;  /**< 对比度设置值 */

/* ==================== 私有函数声明 ==================== */
static void                   ST75161_Delay               ( __IO uint32_t nCount );
static void                   ST75161_GPIO_Config         ( void );
static void                   ST75161_REG_Config          ( void );
static void                   ST75161_SetCursor           ( uint16_t usX, uint16_t usY );
static __inline void          ST75161_FillColor           ( uint32_t ulAmout_Point, uint16_t usColor );
static uint16_t               ST75161_Read_PixelData      ( void );

/**
 * @brief  8位数据输出函数
 * @param  x 要输出的8位数据
 * @retval 无
 */
__inline void data_out(uint16_t x)
{
    GPIO_WriteBit(ST75161_D0_PORT, ST75161_D0_PIN, (BitAction)(x >> 0 & 0x0001));
    GPIO_WriteBit(ST75161_D1_PORT, ST75161_D1_PIN, (BitAction)(x >> 1 & 0x0001));
    GPIO_WriteBit(ST75161_D2_PORT, ST75161_D2_PIN, (BitAction)(x >> 2 & 0x0001));
    GPIO_WriteBit(ST75161_D3_PORT, ST75161_D3_PIN, (BitAction)(x >> 3 & 0x0001));
    GPIO_WriteBit(ST75161_D4_PORT, ST75161_D4_PIN, (BitAction)(x >> 4 & 0x0001));
    GPIO_WriteBit(ST75161_D5_PORT, ST75161_D5_PIN, (BitAction)(x >> 5 & 0x0001));
    GPIO_WriteBit(ST75161_D6_PORT, ST75161_D6_PIN, (BitAction)(x >> 6 & 0x0001));
    GPIO_WriteBit(ST75161_D7_PORT, ST75161_D7_PIN, (BitAction)(x >> 7 & 0x0001));	
}

/**
 * @brief  发送命令到ST75161
 * @param  usCmd 要发送的命令值
 * @retval 无
 */
__inline void comm_out(uint16_t usCmd)
{
    ST75161_CS_CLR;      // 片选使能
    ST75161_DC_CLR;      // 命令模式
    ST75161_RD_SET;      // 读禁止
    data_out(usCmd);     // 输出命令
    ST75161_WR_CLR;      // 写开始
    ST75161_WR_SET;      // 写结束	
    ST75161_CS_SET;      // 片选释放
}

/**
 * @brief  发送数据到ST75161
 * @param  usData 要发送的数据值
 * @retval 无
 */
__inline void DATA_WR(uint16_t usData)
{
    ST75161_CS_CLR;      // 片选使能
    ST75161_DC_SET;      // 数据模式
    ST75161_RD_SET;      // 读禁止
    data_out(usData);    // 输出数据
    ST75161_WR_CLR;      // 写开始
    ST75161_WR_SET;      // 写结束	
    ST75161_CS_SET;      // 片选释放
}

/**
 * @brief  向ST75161写入数据（16位模式，用于兼容）
 * @param  usData 要写入的数据
 * @retval 无
 */
__inline void ST75161_Write_Data(uint16_t usData)
{
    *(__IO uint16_t *)(macFSMC_Addr_ST75161_DATA) = usData >> 8;
    *(__IO uint16_t *)(macFSMC_Addr_ST75161_DATA) = usData;
}

/**
 * @brief  从ST75161读取数据
 * @param  无
 * @retval 读取到的数据
 */
__inline uint16_t ST75161_Read_Data(void)
{
    return (*(__IO uint16_t *)(macFSMC_Addr_ST75161_DATA));
}

/**
 * @brief  ST75161延时函数
 * @param  nCount 延时计数值
 * @retval 无
 */
static void ST75161_Delay(__IO uint32_t nCount)
{
    for(; nCount != 0; nCount--);
}

/**
 * @brief  初始化ST75161的GPIO引脚
 * @param  无
 * @retval 无
 */
static void ST75161_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 使能各IO时钟 */
    RCC_APB2PeriphClockCmd(ST75161_CS_CLK | ST75161_DC_CLK | ST75161_WR_CLK |
                           ST75161_RD_CLK | ST75161_D0_CLK | ST75161_D1_CLK | 
                           ST75161_D2_CLK | ST75161_D3_CLK | ST75161_D4_CLK | 
                           ST75161_D5_CLK | ST75161_D6_CLK | ST75161_D7_CLK, ENABLE);
    
    /* 配置数据引脚为推挽输出 */	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    
    GPIO_InitStructure.GPIO_Pin = ST75161_D0_PIN;
    GPIO_Init(ST75161_D0_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_D1_PIN;
    GPIO_Init(ST75161_D1_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_D2_PIN;
    GPIO_Init(ST75161_D2_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_D3_PIN;
    GPIO_Init(ST75161_D3_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_D4_PIN;
    GPIO_Init(ST75161_D4_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_D5_PIN;
    GPIO_Init(ST75161_D5_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_D6_PIN;
    GPIO_Init(ST75161_D6_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_D7_PIN;
    GPIO_Init(ST75161_D7_PORT, &GPIO_InitStructure);
    
    /* 配置控制引脚 */
    GPIO_InitStructure.GPIO_Pin = ST75161_RD_PIN;
    GPIO_Init(ST75161_RD_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_WR_PIN;
    GPIO_Init(ST75161_WR_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_CS_PIN;
    GPIO_Init(ST75161_CS_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ST75161_DC_PIN;
    GPIO_Init(ST75161_DC_PORT, &GPIO_InitStructure);
    
    /* 配置LCD复位RST控制引脚 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    macST75161_RST_APBxClock_FUN(macST75161_RST_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = macST75161_RST_PIN;
    GPIO_Init(macST75161_RST_PORT, &GPIO_InitStructure);
    
    /* IO口初始状态初始化 */
    GPIO_SetBits(macST75161_RST_PORT, macST75161_RST_PIN);
}

/**
 * @brief  LCD FSMC模式配置
 * @param  无
 * @retval 无
 */
static void ST75161_FSMC_Config(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p;
    
    /* 使能FSMC时钟 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    
    p.FSMC_AddressSetupTime      = 0x02;  // 地址建立时间
    p.FSMC_AddressHoldTime       = 0x00;  // 地址保持时间
    p.FSMC_DataSetupTime         = 0x05;  // 数据建立时间
    p.FSMC_BusTurnAroundDuration = 0x00;
    p.FSMC_CLKDivision           = 0x00;
    p.FSMC_DataLatency           = 0x00;
    p.FSMC_AccessMode            = FSMC_AccessMode_B;  // 模式B
    
    FSMC_NORSRAMInitStructure.FSMC_Bank                  = macFSMC_Bank1_NORSRAMx;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType            = FSMC_MemoryType_NOR;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode              = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = &p;
    
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    
    /* 使能 FSMC_Bank1_NORSRAMx */
    FSMC_NORSRAMCmd(macFSMC_Bank1_NORSRAMx, ENABLE);
}

/**
 * @brief  初始化ST75161寄存器配置
 * @param  无
 * @retval 无
 */
static void ST75161_REG_Config(void)
{
    comm_out(0x31);      // EXT=1
    comm_out(0xD7);      // Disable Auto Read
    DATA_WR(0x9F);
    
    comm_out(0xE0);      // Enable OTP Read
    DATA_WR(0x00);
    
    ST75161_Delay(10);
    comm_out(0xE3);      // OTP Up-Load
    
    ST75161_Delay(20);
    comm_out(0xE1);      // OTP Control Out
    
    comm_out(0x30);      // EXT=0
    comm_out(0x94);      // Sleep out
    comm_out(0xAE);      // Display off
    ST75161_Delay(50);
    
    comm_out(0x20);      // Power control
    DATA_WR(0x0B);       // D0=regulator ; D1=follower ; D3=booste, on:1 off:0
    
    comm_out(0x81);      // EV control
    DATA_WR(Set_Contrast & 0x3F);   // VPR[5-0]
    DATA_WR(Set_Contrast >> 6);     // VPR[8-6]
    
    comm_out(0x31);      // EXT=1
    
    comm_out(0x20);      // Set Gray Scale Level
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0x17);
    DATA_WR(0x17);
    DATA_WR(0x17);
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0x1D);
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0x1D);
    DATA_WR(0x1D);
    DATA_WR(0x1D);
    DATA_WR(0x00);
    DATA_WR(0x00);
    
    comm_out(0x32);      // Analog Circuit Set
    DATA_WR(0x00);
    DATA_WR(0x01);       // Booster Efficiency =6KHz
    DATA_WR(0x02);       // 1/12 Bias
    
    comm_out(0x51);      // Booster Level x10
    DATA_WR(0xFB);
    
    comm_out(0x30);      // EXT=0
    
    comm_out(0xF0);      // Display Mode
    DATA_WR(0x10);       // 10H=Monochrome Mode,11H=4Gray
    
    comm_out(0xCA);      // Display Control
    DATA_WR(0x00);       // CL Dividing Ratio? Not Divide
    DATA_WR(0x9F);       // Duty Set 160
    DATA_WR(0x25);       // Frame Inversion
    
    comm_out(0xBC);      // Data Scan Direction
    DATA_WR(0x00);
    
    comm_out(0xA6);      // Normal display
    
    comm_out(0x31);      // EXT=1
    
    comm_out(0xF0);      // Frame
    DATA_WR(0x0D);
    DATA_WR(0x0D);
    DATA_WR(0x0D);
    DATA_WR(0x0D);
    
    comm_out(0xF2);      // Temperature Range
    DATA_WR(0x05);       // TA=-35
    DATA_WR(0x23);       // TB=-5
    DATA_WR(0x40);       // TC=24
    
    comm_out(0xF4);      // Temperature Gradient Compensation
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0x00);
    DATA_WR(0xB9);
    DATA_WR(0xFF);
    
    comm_out(0x31);      // EXT=1
    comm_out(0x40);      // Internal Power Supply
    
    clear_ICON_DDRAM();
    clealddram();
    
    comm_out(0xAF);      // Display on
}

/**
 * @brief  ST75161初始化函数
 * @param  无
 * @retval 无
 */
void ST75161_Init(void)
{
    ST75161_GPIO_Config();
    ST75161_FSMC_Config();
    ST75161_Rst();
    ST75161_REG_Config();
}

/**
 * @brief  ST75161背光LED控制
 * @param  enumState ENABLE-开背光, DISABLE-关背光
 * @retval 无
 */
void ST75161_BackLed_Control(FunctionalState enumState)
{
    if(enumState)
        GPIO_ResetBits(macST75161_BK_PORT, macST75161_BK_PIN);
    else
        GPIO_SetBits(macST75161_BK_PORT, macST75161_BK_PIN);
}

/**
 * @brief  ST75161硬件复位
 * @param  无
 * @retval 无
 */
void ST75161_Rst(void)
{
    GPIO_SetBits(macST75161_RST_PORT, macST75161_RST_PIN);
    ST75161_Delay(0xAFFf << 2);
    
    GPIO_ResetBits(macST75161_RST_PORT, macST75161_RST_PIN);  // 低电平复位
    ST75161_Delay(0xAFFf << 2);
    
    GPIO_SetBits(macST75161_RST_PORT, macST75161_RST_PIN);
    ST75161_Delay(0xAFFf << 2);
    ST75161_Delay(0xAFFf << 2);
    ST75161_Delay(0xAFFf << 2);
    ST75161_Delay(0xAFFf << 2);
    ST75161_Delay(0xAFFf << 2);
}

/**
 * @brief  设置ST75161的GRAM扫描方向
 * @param  ucOption 扫描方向选项(1-4)
 * @retval 无
 */
void ST75161_GramScan(uint8_t ucOption)
{
    switch(ucOption)
    {
        case 1:  // 左上角->右下角
            comm_out(0x36);
            data_out(0xC8);
            comm_out(macCMD_SetCoordinateX);
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0xEF);
            comm_out(macCMD_SetCoordinateY);
            data_out(0x00); data_out(0x00);
            data_out(0x01); data_out(0x3F);
            break;
        
        case 2:  // 右下角->左上角
            comm_out(0x36);
            data_out(0x68);
            comm_out(macCMD_SetCoordinateX);
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0xEF);
            comm_out(macCMD_SetCoordinateY);
            data_out(0x00); data_out(0x00);
            data_out(0x01); data_out(0x3F);
            break;
        
        case 3:  // 左下角->右上角
            comm_out(0x36);
            data_out(0x28);
            comm_out(macCMD_SetCoordinateX);
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0xEF);
            comm_out(macCMD_SetCoordinateY);
            data_out(0x00); data_out(0x00);
            data_out(0x01); data_out(0x3F);
            break;
        
        case 4:  // 右上角->左下角
            comm_out(0x36);
            data_out(0x48);
            comm_out(macCMD_SetCoordinateX);
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0xEF);
            comm_out(macCMD_SetCoordinateY);
            data_out(0x00); data_out(0x00);
            data_out(0x01); data_out(0x3F);
            break;
    }
    
    comm_out(macCMD_SetPixel);
}

/**
 * @brief  在ST75161显示区域上打开一个窗口
 * @param  usX 窗口起始X坐标
 * @param  usY 窗口起始Y坐标
 * @param  usWidth 窗口宽度
 * @param  usHeight 窗口高度
 * @retval 无
 */
void ST75161_OpenWindow(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{
    comm_out(macCMD_SetCoordinateX);
    data_out((usX + 0) >> 8);
    data_out((usX + 0) & 0xFF);
    data_out((usX + 0 + usWidth - 1) >> 8);
    data_out((usX + 0 + usWidth - 1) & 0xFF);
    
    comm_out(macCMD_SetCoordinateY);
    data_out((usY + 0) >> 8);
    data_out((usY + 0) & 0xFF);
    data_out((usY + 0 + usHeight - 1) >> 8);
    data_out((usY + 0 + usHeight - 1) & 0xFF);
}

/**
 * @brief  设定ST75161的光标位置
 * @param  usX 光标X坐标
 * @param  usY 光标Y坐标
 * @retval 无
 */
static void ST75161_SetCursor(uint16_t usX, uint16_t usY)
{
    ST75161_OpenWindow(usX, usY, 1, 1);
}

/**
 * @brief  向ST75161显示区域填充颜色
 * @param  ulAmout_Point 需要填充的像素点数
 * @param  usColor 填充颜色
 * @retval 无
 */
static __inline void ST75161_FillColor(uint32_t ulAmout_Point, uint16_t usColor)
{
    uint32_t i = 0;
    
    comm_out(macCMD_SetPixel);
    
    for(i = 0; i < ulAmout_Point; i++)
        ST75161_Write_Data(usColor);
}

/**
 * @brief  区域清屏填充
 * @param  usX 起始X坐标
 * @param  usY 起始Y坐标
 * @param  usWidth 宽度
 * @param  usHeight 高度
 * @param  usColor 填充颜色
 * @retval 无
 */
void ST75161_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint16_t usColor)
{
    ST75161_OpenWindow(usX, usY, usWidth, usHeight);
    ST75161_FillColor(usWidth * usHeight, usColor);
}

/**
 * @brief  绘制像素点
 * @param  usX X坐标
 * @param  usY Y坐标
 * @param  usColor 颜色
 * @retval 无
 */
void ST75161_SetPointPixel(uint16_t usX, uint16_t usY, uint16_t usColor)
{
    if((usX < macST75161_DispWindow_COLUMN) && (usY < macST75161_DispWindow_PAGE))
    {
        ST75161_SetCursor(usX, usY);
        ST75161_FillColor(1, usColor);
    }
}

/**
 * @brief  读取像素数据
 * @param  无
 * @retval 像素颜色值
 */
static uint16_t ST75161_Read_PixelData(void)
{
    uint16_t usR = 0, usG = 0, usB = 0;
    
    comm_out(0x2E);
    usR = ST75161_Read_Data();
    usR = ST75161_Read_Data();
    usB = ST75161_Read_Data();
    usG = ST75161_Read_Data();
    
    return (((usR >> 11) << 11) | ((usG >> 10) << 5) | (usB >> 11));
}

/**
 * @brief  获取指定坐标的像素颜色
 * @param  usX X坐标
 * @param  usY Y坐标
 * @retval 像素颜色值
 */
uint16_t ST75161_GetPointPixel(uint16_t usX, uint16_t usY)
{
    uint16_t usPixelData;
    
    ST75161_SetCursor(usX, usY);
    usPixelData = ST75161_Read_PixelData();
    
    return usPixelData;
}

/**
 * @brief  绘制线段（Bresenham算法）
 * @param  usX1 起点X
 * @param  usY1 起点Y
 * @param  usX2 终点X
 * @param  usY2 终点Y
 * @param  usColor 颜色
 * @retval 无
 */
void ST75161_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2, uint16_t usColor)
{
    uint16_t us;
    uint16_t usX_Current, usY_Current;
    
    int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance;
    int32_t lIncrease_X, lIncrease_Y;
    
    lDelta_X = usX2 - usX1;
    lDelta_Y = usY2 - usY1;
    
    usX_Current = usX1;
    usY_Current = usY1;
    
    if(lDelta_X > 0)
        lIncrease_X = 1;
    else if(lDelta_X == 0)
        lIncrease_X = 0;
    else
    {
        lIncrease_X = -1;
        lDelta_X = -lDelta_X;
    }
    
    if(lDelta_Y > 0)
        lIncrease_Y = 1;
    else if(lDelta_Y == 0)
        lIncrease_Y = 0;
    else
    {
        lIncrease_Y = -1;
        lDelta_Y = -lDelta_Y;
    }
    
    if(lDelta_X > lDelta_Y)
        lDistance = lDelta_X;
    else
        lDistance = lDelta_Y;
    
    for(us = 0; us <= lDistance + 1; us++)
    {
        ST75161_SetPointPixel(usX_Current, usY_Current, usColor);
        
        lError_X += lDelta_X;
        lError_Y += lDelta_Y;
        
        if(lError_X > lDistance)
        {
            lError_X -= lDistance;
            usX_Current += lIncrease_X;
        }
        
        if(lError_Y > lDistance)
        {
            lError_Y -= lDistance;
            usY_Current += lIncrease_Y;
        }
    }
}

/**
 * @brief  绘制矩形
 * @param  usX_Start 起始X
 * @param  usY_Start 起始Y
 * @param  usWidth 宽度
 * @param  usHeight 高度
 * @param  usColor 颜色
 * @param  ucFilled 是否填充
 * @retval 无
 */
void ST75161_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, 
                           uint16_t usHeight, uint16_t usColor, uint8_t ucFilled)
{
    if(ucFilled)
        ST75161_Clear(usX_Start, usY_Start, usWidth, usHeight, usColor);
    else
    {
        ST75161_DrawLine(usX_Start, usY_Start, usX_Start + usWidth - 1, usY_Start, usColor);
        ST75161_DrawLine(usX_Start, usY_Start + usHeight - 1, usX_Start + usWidth - 1, usY_Start + usHeight - 1, usColor);
        ST75161_DrawLine(usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1, usColor);
        ST75161_DrawLine(usX_Start + usWidth - 1, usY_Start, usX_Start + usWidth - 1, usY_Start + usHeight - 1, usColor);
    }
}

/**
 * @brief  绘制圆（Bresenham算法）
 * @param  usX_Center 圆心X
 * @param  usY_Center 圆心Y
 * @param  usRadius 半径
 * @param  usColor 颜色
 * @param  ucFilled 是否填充
 * @retval 无
 */
void ST75161_DrawCircle(uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, 
                        uint16_t usColor, uint8_t ucFilled)
{
    int16_t sCurrentX, sCurrentY;
    int16_t sError;
    
    sCurrentX = 0;
    sCurrentY = usRadius;
    sError = 3 - (usRadius << 1);
    
    while(sCurrentX <= sCurrentY)
    {
        int16_t sCountY;
        
        if(ucFilled)
            for(sCountY = sCurrentX; sCountY <= sCurrentY; sCountY++)
            {
                ST75161_SetPointPixel(usX_Center + sCurrentX, usY_Center + sCountY, usColor);
                ST75161_SetPointPixel(usX_Center - sCurrentX, usY_Center + sCountY, usColor);
                ST75161_SetPointPixel(usX_Center - sCountY, usY_Center + sCurrentX, usColor);
                ST75161_SetPointPixel(usX_Center - sCountY, usY_Center - sCurrentX, usColor);
                ST75161_SetPointPixel(usX_Center - sCurrentX, usY_Center - sCountY, usColor);
                ST75161_SetPointPixel(usX_Center + sCurrentX, usY_Center - sCountY, usColor);
                ST75161_SetPointPixel(usX_Center + sCountY, usY_Center - sCurrentX, usColor);
                ST75161_SetPointPixel(usX_Center + sCountY, usY_Center + sCurrentX, usColor);
            }
        else
        {
            ST75161_SetPointPixel(usX_Center + sCurrentX, usY_Center + sCurrentY, usColor);
            ST75161_SetPointPixel(usX_Center - sCurrentX, usY_Center + sCurrentY, usColor);
            ST75161_SetPointPixel(usX_Center - sCurrentY, usY_Center + sCurrentX, usColor);
            ST75161_SetPointPixel(usX_Center - sCurrentY, usY_Center - sCurrentX, usColor);
            ST75161_SetPointPixel(usX_Center - sCurrentX, usY_Center - sCurrentY, usColor);
            ST75161_SetPointPixel(usX_Center + sCurrentX, usY_Center - sCurrentY, usColor);
            ST75161_SetPointPixel(usX_Center + sCurrentY, usY_Center - sCurrentX, usColor);
            ST75161_SetPointPixel(usX_Center + sCurrentY, usY_Center + sCurrentX, usColor);
        }
        
        sCurrentX++;
        
        if(sError < 0)
            sError += 4 * sCurrentX + 6;
        else
        {
            sError += 10 + 4 * (sCurrentX - sCurrentY);
            sCurrentY--;
        }
    }
}

/**
 * @brief  显示英文字符
 * @param  usX X坐标
 * @param  usY Y坐标
 * @param  cChar 字符
 * @param  usColor_Background 背景色
 * @param  usColor_Foreground 前景色
 * @retval 无
 */
void ST75161_DispChar_EN(uint16_t usX, uint16_t usY, const char cChar, 
                         uint16_t usColor_Background, uint16_t usColor_Foreground)
{
    uint8_t ucTemp, ucRelativePositon, ucPage, ucColumn;
    
    ucRelativePositon = cChar - ' ';
    
    ST75161_OpenWindow(usX, usY, macWIDTH_EN_CHAR, macHEIGHT_EN_CHAR);
    comm_out(macCMD_SetPixel);
    
    for(ucPage = 0; ucPage < macHEIGHT_EN_CHAR; ucPage++)
    {
        ucTemp = ucAscii_1608[ucRelativePositon][ucPage];
        
        for(ucColumn = 0; ucColumn < macWIDTH_EN_CHAR; ucColumn++)
        {
            if(ucTemp & 0x01)
                ST75161_Write_Data(usColor_Foreground);
            else
                ST75161_Write_Data(usColor_Background);
            
            ucTemp >>= 1;
        }
    }
}

/**
 * @brief  显示英文字符串
 * @param  usX X坐标
 * @param  usY Y坐标
 * @param  pStr 字符串指针
 * @param  usColor_Background 背景色
 * @param  usColor_Foreground 前景色
 * @retval 无
 */
void ST75161_DispString_EN(uint16_t usX, uint16_t usY, const char *pStr, 
                           uint16_t usColor_Background, uint16_t usColor_Foreground)
{
    while(*pStr != '\0')
    {
        if((usX - macST75161_DispWindow_X_Star + macWIDTH_EN_CHAR) > macST75161_DispWindow_COLUMN)
        {
            usX = macST75161_DispWindow_X_Star;
            usY += macHEIGHT_EN_CHAR;
        }
        
        if((usY - macST75161_DispWindow_Y_Star + macHEIGHT_EN_CHAR) > macST75161_DispWindow_PAGE)
        {
            usX = macST75161_DispWindow_X_Star;
            usY = macST75161_DispWindow_Y_Star;
        }
        
        ST75161_DispChar_EN(usX, usY, *pStr, usColor_Background, usColor_Foreground);
        
        pStr++;
        usX += macWIDTH_EN_CHAR;
    }
}

/**
 * @brief  清空ICON DDRAM
 * @param  无
 * @retval 无
 */
void clear_ICON_DDRAM(void)
{
    int j;
    comm_out(0x30);
    comm_out(0x77);  // Enable ICON RAM
    comm_out(0x15);  // Column Address Setting
    DATA_WR(0x00);   // SEG0 -> SEG255
    DATA_WR(0x9F);
    
    comm_out(0x5C);
    for(j = 0; j < 256; j++)
    {
        DATA_WR(0x00);
    }
    comm_out(0x76);  // Disable ICON RAM
}

/**
 * @brief  清空显示DDRAM
 * @param  无
 * @retval 无
 */
void clealddram(void)
{
    int i, j;
    comm_out(0x30);
    comm_out(0xF0);      // Display Mode
    DATA_WR(0x10);       // Monochrome Mode
    comm_out(0x15);
    DATA_WR(0x00);
    DATA_WR(0x9F);
    comm_out(0x75);
    DATA_WR(0x00);
    DATA_WR(0x13);
    comm_out(0x5C);
    for(i = 0; i < 20; i++)
    {
        for(j = 0; j < 160; j++)
        {
            DATA_WR(0x00);
        }
    }
}

/**
 * @brief  显示图形数据
 * @param  gph 图形数据指针
 * @retval 无
 */
void disp_graphics1(const unsigned char *gph)
{
    int i, k;
    
    comm_out(0x30);      // Extension Command 1
    comm_out(0x15);      // Column Address Setting
    DATA_WR(0x00);       // SEG0 -> SEG160
    DATA_WR(0x9F);
    comm_out(0x75);      // Page Address Setting
    DATA_WR(0x00);       // COM0 -> COM160
    DATA_WR(0x9F);
    comm_out(0x5C);
    
    for(i = 0; i < 20; i++)
    {
        for(k = 0; k < 160; k++)
        {
            DATA_WR(*gph);
            gph++;
        }
    }
}

/**
 * @brief  显示颜色条测试图案
 * @param  无
 * @retval 无
 */
void disp_color_table(void)
{
    int ii, jj, kk;
    int cb[8] = {0xF800, 0x07E0, 0x001F, 0xFFE0, 0xF81F, 0x07FF, 0xFFFF, 0x0000};
    
    ST75161_OpenWindow(0, 0, 160, 160);
    comm_out(macCMD_SetPixel);
    
    for(ii = 0; ii < 160; ii++)
    {
        for(jj = 0; jj < 7; jj++)
        {
            for(kk = 0; kk < 160 / 8; kk++)
                ST75161_Write_Data(cb[jj]);
        }
        
        for(kk = 0; kk < (160 - 30 * 7); kk++)
            ST75161_Write_Data(cb[7]);
    }
}

/**
 * @brief  显示9宫格棋盘格测试图案
 * @param  无
 * @retval 无
 */
void DISP9W(void)
{
    unsigned int i, j;
    unsigned int a, b;
    
    ST75161_OpenWindow(0, 0, 160, 160);
    comm_out(macCMD_SetPixel);
    
    a = 160 / 5;
    b = 160 / 5;
    
    for(i = 0; i < 160; i++)
    {
        for(j = 0; j < 160; j++)
        {
            if(i < a || (i >= a * 2 && i < a * 3) || i >= a * 4)
            {
                if(j < b || (j >= b * 2 && j < b * 3) || j >= b * 4)
                    ST75161_Write_Data(0xFFFF);
                else
                    ST75161_Write_Data(0x0000);
            }
            else
            {
                if(j < b || (j >= b * 2 && j < b * 3) || j >= b * 4)
                    ST75161_Write_Data(0x0000);
                else
                    ST75161_Write_Data(0xFFFF);
            }
        }
    }
}

/**
 * @brief  显示边框测试图案
 * @param  无
 * @retval 无
 */
void side(void)
{
    int ii, jj;
    
    ST75161_OpenWindow(0, 0, 160, 160);
    comm_out(macCMD_SetPixel);
    
    for(ii = 0; ii < 160; ii++)
    {
        if(ii == 0)
        {
            for(jj = 0; jj < 160; jj++)
                ST75161_Write_Data(0xFFFF);
        }
        else if(ii == (160 - 1))
        {
            for(jj = 0; jj < 160; jj++)
                ST75161_Write_Data(0xFFFF);
        }
        else
        {
            for(jj = 0; jj < 160; jj++)
            {
                if(jj == 0 || jj == (160 - 1))
                    ST75161_Write_Data(0xFFFF);
                else
                    ST75161_Write_Data(0x0000);
            }
        }
    }
}

/**
 * @brief  显示灰度渐变测试图案
 * @param  无
 * @retval 无
 */
void disp_gray(void)
{
    unsigned int i, j, k;
    
    ST75161_OpenWindow(0, 0, 160, 160);
    comm_out(macCMD_SetPixel);
    
    for(k = 0; k < (160 % 32); k++)
    {
        for(j = 0; j < 160; j++)
        {
            ST75161_Write_Data(0x0000);
        }
    }
    
    for(k = 0; k < 32; k++)
    {
        for(i = 0; i < 160 / 32; i++)
        {
            for(j = 0; j < 160; j++)
            {
                ST75161_Write_Data(RGB24TORGB16(k << 3, k << 3, k << 3));
            }
        }
    }
}
#endif

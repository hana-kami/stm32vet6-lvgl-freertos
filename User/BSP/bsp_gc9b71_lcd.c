/**
 * @file bsp_gc9b71_lcd.c
 * @brief GC9B71 LCD屏幕驱动（SPI接口版本）
 * @note 适用于GC9B71驱动IC的圆形LCD屏幕，使用SPI通信协议
 */

#include "bsp_gc9b71_lcd.h"
#include "bsp_lcd_config.h"

#if (CURRENT_LCD_TYPE == LCD_TYPE_GC9B71)

#include "ascii.h"	

/* 私有函数声明 */
static void                   GC9B71_Delay               ( __IO uint32_t nCount );
static void                   GC9B71_GPIO_Config         ( void );
static void                   GC9B71_REG_Config          ( void );
static void                   GC9B71_SetCursor           ( uint16_t usX, uint16_t usY );
static __inline void          GC9B71_FillColor           ( uint32_t ulAmout_Point, uint32_t usColor );
static uint16_t               GC9B71_Read_PixelData      ( void );

/**
 * @brief  SPI通信延时函数
 * @param  无
 * @retval 无
 */
void spi_delay(void)
{
    volatile int i;
    for(i = 0; i < 10; i++);
}

/**
 * @brief  向GC9B71发送命令（SPI 8位模式）
 * @param  usCmd: 要发送的命令值（8位）
 * @retval 无
 * @note   RS=0表示命令，CS低电平有效
 */
__inline void comm_out(uint16_t usCmd)
{
    int i;

    LCD_CS_CLR;      // 片选使能
    LCD_RS_CLR;      // 命令模式（RS=0）

    for(i = 0; i < 8; i++)
    {
        // 发送数据位（高位在前）
        if(usCmd & 0x80){
            LCD_SDA_SET;{
}
				}
        else{
					LCD_SDA_CLR;
				}
        spi_delay();

        usCmd <<= 1;
        
        // 时钟上升沿锁存数据
        LCD_SCL_CLR;
        spi_delay();
        spi_delay();
        LCD_SCL_SET;
        spi_delay();
        spi_delay();
    }
    LCD_CS_SET;      // 片选释放
    spi_delay();
}

/**
 * @brief  向GC9B71发送数据（SPI 8位模式）
 * @param  usData: 要发送的数据值（8位）
 * @retval 无
 * @note   RS=1表示数据，CS低电平有效
 */
__inline void data_out(uint16_t usData)
{
    int i;

    LCD_CS_CLR;      // 片选使能
    LCD_RS_SET;      // 数据模式（RS=1）

    for(i = 0; i < 8; i++)
    {
        // 发送数据位（高位在前）
        if(usData & 0x80){
            LCD_SDA_SET;
				}
        else{
            LCD_SDA_CLR;
				}
        spi_delay();

        usData <<= 1;
        
        // 时钟上升沿锁存数据
        LCD_SCL_CLR;
        spi_delay();
        spi_delay();
        LCD_SCL_SET;
        spi_delay();
        spi_delay();
    }
    LCD_CS_SET;      // 片选释放
    spi_delay();
}

/**
 * @brief  向GC9B71写入24位数据（用于RGB565颜色数据）
 * @param  usData: 24位颜色数据（RGB565格式存储在低16位）
 * @retval 无
 * @note   RS=1表示数据模式，24位数据一次性发送
 */
__inline void GC9B71_Write_Data(uint32_t usData)
{
    int i;

    LCD_CS_CLR;      // 片选使能
    LCD_RS_SET;      // 数据模式（RS=1）

    for(i = 0; i < 24; i++)
    {
        // 发送数据位（高位在前）
        if(usData & 0x800000){
            LCD_SDA_SET;
				}
        else{
            LCD_SDA_CLR;
				}

        usData <<= 1;
        LCD_SCL_CLR;
        LCD_SCL_SET;
    }
    LCD_CS_SET;      // 片选释放
}

/**
 * @brief  从GC9B71读取数据（当前未实现SPI读取）
 * @param  无
 * @retval 读取到的数据
 * @note   当前返回0，如需读取功能请实现SPI读取
 */
__inline uint16_t GC9B71_Read_Data(void)
{
    return 0;  // SPI读取需要实现双向通信
}

/**
 * @brief  GC9B71延时函数
 * @param  nCount: 延时计数值
 * @retval 无
 */
static void GC9B71_Delay(__IO uint32_t nCount)
{
    for(; nCount != 0; nCount--);
}

/**
 * @brief  初始化GC9B71的GPIO引脚
 * @param  无
 * @retval 无
 * @note   配置SPI通信所需的引脚：CS、RS、SCL、SDA、RST、BK
 */
static void GC9B71_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 使能各IO时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    
    /* 使能FSMC对应的引脚时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);
    
    /* 配置FSMC对应数据引脚, FSMC-D0~D15: PD 14 15 0 1, PE 7 8 9 10 11 12 13 14 15, PD 8 9 10 */	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_8 | GPIO_Pin_9 | 
                                  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11|
                                  GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    /* 配置控制引脚（改为推挽输出模式用于SPI软件模拟）
     * PD4: SDA（数据）
     * PD5: SCL（时钟）
     * PD7: CS（片选）
     * PE11: RS（命令/数据选择）
     */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;   // SDA
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;   // SCL
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  // RS
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* 配置CS引脚 */
    macGC9B71_CS_APBxClock_FUN(macGC9B71_CS_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = macGC9B71_CS_PIN; 
    GPIO_Init(macGC9B71_CS_PORT, &GPIO_InitStructure);  
    
    /* 配置DC/RS引脚 */
    macGC9B71_DC_APBxClock_FUN(macGC9B71_DC_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = macGC9B71_DC_PIN; 
    GPIO_Init(macGC9B71_DC_PORT, &GPIO_InitStructure);
    
    /* 配置LCD复位RST控制引脚 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    macGC9B71_RST_APBxClock_FUN(macGC9B71_RST_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = macGC9B71_RST_PIN; 
    GPIO_Init(macGC9B71_RST_PORT, &GPIO_InitStructure);
    
    /* 配置LCD背光控制引脚BK */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    
    macGC9B71_BK_APBxClock_FUN(macGC9B71_BK_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = macGC9B71_BK_PIN; 
    GPIO_Init(macGC9B71_BK_PORT, &GPIO_InitStructure);
    
    /* IO口初始状态初始化 */
    GPIO_SetBits(macGC9B71_RST_PORT, macGC9B71_RST_PIN);	
    GPIO_SetBits(macGC9B71_CS_PORT, macGC9B71_CS_PIN);
    GPIO_SetBits(macGC9B71_DC_PORT, macGC9B71_DC_PIN);
    
    /* SPI引脚初始电平设置 */
    LCD_CS_SET;
    LCD_RS_SET;
    LCD_SCL_SET;
    LCD_SDA_SET;
}

/**
 * @brief  初始化GC9B71寄存器配置
 * @param  无
 * @retval 无
 * @note   这是GC9B71屏幕专用的初始化序列
 */
static void GC9B71_REG_Config(void)
{
    // GC9B71初始化序列
    comm_out(0xfe);	
    comm_out(0xef);

    comm_out(0x80);	
    data_out(0x11);

    comm_out(0x81);	
    data_out(0x30);

    comm_out(0x82);	
    data_out(0x09);
    
    comm_out(0x83);	
    data_out(0x03);
    
    comm_out(0x84);	
    data_out(0x30);  // b4 en	

    comm_out(0x89);	
    data_out(0x18);
    
    comm_out(0x8A);	
    data_out(0x40);

    comm_out(0x8B);	
    data_out(0x0A);
    
    comm_out(0x3a);  	
    data_out(0x66);    // 16位色深模式

    comm_out(0x36);    // 内存访问控制
    data_out(0x00);

    comm_out(0xEC);
    data_out(0x00);
                
    comm_out(0x74);
    data_out(0x01);
    data_out(0xBf);
    data_out(0x00);
    data_out(0x00);
    data_out(0x00);
    data_out(0x00);
    
    comm_out(0x98);
    data_out(0x3E);
    comm_out(0x99);
    data_out(0x3E);
                
    comm_out(0xA1);
    data_out(0x01);
    data_out(0x04);
    
    comm_out(0xA2);
    data_out(0x01);
    data_out(0x04);
    
    comm_out(0xCB);
    data_out(0x02);
    
    comm_out(0x7C);
    data_out(0xB6);
    data_out(0x24);
    
    comm_out(0xAC);
    data_out(0x44);

    comm_out(0xF6);
    data_out(0x80);

    comm_out(0xB5);
    data_out(0x0c);
    data_out(0x0c);
    
    comm_out(0xEB);
    data_out(0x01); 
    data_out(0xdF); 

    comm_out(0x60);
    data_out(0x38); 
    data_out(0x0c);
    data_out(0x13); 
    data_out(0x56);

    comm_out(0x63);
    data_out(0x38); 
    data_out(0x0e); 
    data_out(0x13); 
    data_out(0x56);

    comm_out(0x61);
    data_out(0x3B); 
    data_out(0xDa); 
    data_out(0x58); 
    data_out(0x38);

    comm_out(0x62);
    data_out(0x3B); 
    data_out(0xDa); 
    data_out(0x58); 
    data_out(0x38);

    comm_out(0x64);
    data_out(0x38); 
    data_out(0x10); 
    data_out(0x73); 
    data_out(0xD6); 
    data_out(0x13); 
    data_out(0x56);

    comm_out(0x66);
    data_out(0x38); 
    data_out(0x11); 
    data_out(0x73); 
    data_out(0xD7); 
    data_out(0x13); 
    data_out(0x56); 

    comm_out(0x68);
    data_out(0x00); 
    data_out(0x0B);
    data_out(0xDA);
    data_out(0x0B); 
    data_out(0xDA); 
    data_out(0x1C);
    data_out(0x1C); 

    comm_out(0x69);
    data_out(0x00); 
    data_out(0x0B); 
    data_out(0xE6); 
    data_out(0x0B); 
    data_out(0xE6); 
    data_out(0x1C); 
    data_out(0x1C); 

    comm_out(0x6A);
    data_out(0x15); 
    data_out(0x00); 

    comm_out(0x6E);
    data_out(0x07); 
    data_out(0x01); 
    data_out(0x1A); 
    data_out(0x00); 
    data_out(0x0A); 
    data_out(0x09); 
    data_out(0x0C); 
    data_out(0x0B); 
    data_out(0x03); 
    data_out(0x19); 
    data_out(0x1E); 
    data_out(0x1D); 
    data_out(0x00); 
    data_out(0x00); 
    data_out(0x00); 
    data_out(0x00); 
    data_out(0x00); 
    data_out(0x00); 
    data_out(0x00); 
    data_out(0x00); 
    data_out(0x1D); 
    data_out(0x1E); 
    data_out(0x19); 
    data_out(0x05); 
    data_out(0x13); 
    data_out(0x14); 
    data_out(0x11); 
    data_out(0x12); 
    data_out(0x00); 
    data_out(0x1A);
    data_out(0x02); 
    data_out(0x08); 

    comm_out(0x6C);
    data_out(0xCC); 
    data_out(0x0C); 
    data_out(0xCC); 
    data_out(0x84); 
    data_out(0xCC); 
    data_out(0x04); 
    data_out(0x5F); 

    comm_out(0x7D);
    data_out(0x72);

    comm_out(0x70);
    data_out(0x02);
    data_out(0x03);
    data_out(0x09);
    data_out(0x07);
    data_out(0x09);
    data_out(0x03);
    data_out(0x09);
    data_out(0x07);
    data_out(0x09); 
    data_out(0x03);

    comm_out(0x90);
    data_out(0x06);
    data_out(0x06);
    data_out(0x05);
    data_out(0x06);

    comm_out(0x93);
    data_out(0x45);
    data_out(0xFF);
    data_out(0x00);

    comm_out(0xC3);
    data_out(0x15);
    comm_out(0xC4);
    data_out(0x36);
    comm_out(0xC9);
    data_out(0x3d);

    comm_out(0xF0);   
    data_out(0x4D);
    data_out(0x10);
    data_out(0x0A);
    data_out(0x0A);
    data_out(0x06);
    data_out(0x34);

    comm_out(0xF2);
    data_out(0x4D);
    data_out(0x10);
    data_out(0x0a);
    data_out(0x0A);
    data_out(0x06);
    data_out(0x33);

    comm_out(0xF1);
    data_out(0x48);
    data_out(0x90);
    data_out(0x93);
    data_out(0x2D);
    data_out(0x2F);
    data_out(0x4F);

    comm_out(0xF3);
    data_out(0x48);
    data_out(0x70);
    data_out(0x73);
    data_out(0x2D);
    data_out(0x2F);
    data_out(0x4F);

    comm_out(0xF9);   
    data_out(0x20);

    comm_out(0xBE);
    data_out(0x11); 

    comm_out(0xFB);
    data_out(0x00); 
    data_out(0x00); 

    comm_out(0xB4);  // te width
    data_out(0x0a); 

    comm_out(0x35);
    data_out(0x00); 

    comm_out(0xfe);
    comm_out(0xee);

    comm_out(0x11);  // 退出睡眠模式
    GC9B71_Delay(120);	

    comm_out(0x29);  // 开启显示
    GC9B71_Delay(120);	
}

/**
 * @brief  GC9B71初始化函数
 * @param  无
 * @retval 无
 * @note   使用LCD前必须调用此函数
 */
void GC9B71_Init(void)
{
    GC9B71_GPIO_Config();
    GC9B71_Rst();
    GC9B71_REG_Config();
}

/**
 * @brief  GC9B71背光LED控制
 * @param  enumState: 是否使能背光LED
 *   @arg ENABLE: 使能背光LED
 *   @arg DISABLE: 禁能背光LED
 * @retval 无
 */
void GC9B71_BackLed_Control(FunctionalState enumState)
{
    if(enumState)
        GPIO_ResetBits(macGC9B71_BK_PORT, macGC9B71_BK_PIN);
    else
        GPIO_SetBits(macGC9B71_BK_PORT, macGC9B71_BK_PIN);
}

/**
 * @brief  GC9B71硬件复位
 * @param  无
 * @retval 无
 */
void GC9B71_Rst(void)
{	
    GPIO_SetBits(macGC9B71_RST_PORT, macGC9B71_RST_PIN);		 	 
    GC9B71_Delay(0xAFFf << 2);
    
    GPIO_ResetBits(macGC9B71_RST_PORT, macGC9B71_RST_PIN);  // 低电平复位
    GC9B71_Delay(0xAFFf << 2);				   
    
    GPIO_SetBits(macGC9B71_RST_PORT, macGC9B71_RST_PIN);		 	 
    GC9B71_Delay(0xAFFf << 2); 	
    GC9B71_Delay(0xAFFf << 2); 
    GC9B71_Delay(0xAFFf << 2); 
    GC9B71_Delay(0xAFFf << 2); 
    GC9B71_Delay(0xAFFf << 2); 
}

/**
 * @brief  设置GC9B71的GRAM扫描方向
 * @param  ucOption: 选择GRAM的扫描方向
 *   @arg 1: 左上角->右下角
 *   @arg 2: 右下角->左上角
 *   @arg 3: 左下角->右上角
 *   @arg 4: 右上角->左下角
 * @retval 无
 */
void GC9B71_GramScan(uint8_t ucOption)
{	
    switch(ucOption)
    {
        case 1:
            comm_out(macCMD_SetCoordinateX); 
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0xEF);
            comm_out(macCMD_SetCoordinateY); 
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0xEF);
            break;
        
        case 2:
            comm_out(0x36); 
            data_out(0x68);	
            comm_out(macCMD_SetCoordinateX); 
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0x7F);	
            comm_out(macCMD_SetCoordinateY); 
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0x9F);		
            break;
        
        case 3:
            comm_out(0x36); 
            data_out(0x28);	
            comm_out(macCMD_SetCoordinateX); 
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0x7F);	
            comm_out(macCMD_SetCoordinateY); 
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0x9F);			
            break;

        case 4:
            comm_out(0x36); 
            data_out(0x48);	
            comm_out(macCMD_SetCoordinateX); 
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0x7F);	
            comm_out(macCMD_SetCoordinateY); 
            data_out(0x00); data_out(0x00);
            data_out(0x00); data_out(0x9F);		
            break;
    }
    
    comm_out(macCMD_SetPixel);  // 写GRAM开始
}

/**
 * @brief  在GC9B71显示区域上打开一个窗口
 * @param  usX: 窗口的起始X坐标
 * @param  usY: 窗口的起始Y坐标
 * @param  usWidth: 窗口的宽度
 * @param  usHeight: 窗口的高度
 * @retval 无
 */
void GC9B71_OpenWindow(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{	
    usX += 20;  // 偏移量调整
    
    comm_out(macCMD_SetCoordinateX);
    data_out((usX) >> 8);
    data_out((usX) & 0xff);
    data_out((usX + usWidth - 1) >> 8);
    data_out((usX + usWidth - 1) & 0xff);

    comm_out(macCMD_SetCoordinateY);
    data_out((usY) >> 8);
    data_out((usY) & 0xff);
    data_out((usY + usHeight - 1) >> 8);
    data_out((usY + usHeight - 1) & 0xff);
}

/**
 * @brief  设定GC9B71的光标位置
 * @param  usX: 光标X坐标
 * @param  usY: 光标Y坐标
 * @retval 无
 */
static void GC9B71_SetCursor(uint16_t usX, uint16_t usY)	
{
    GC9B71_OpenWindow(usX, usY, 1, 1);
}

/**
 * @brief  向GC9B71显示区域填充某种颜色的像素点
 * @param  ulAmout_Point: 需要填充颜色的像素点的数目
 * @param  usColor: 填充颜色（24位RGB格式）
 * @retval 无
 */
static __inline void GC9B71_FillColor(uint32_t ulAmout_Point, uint32_t usColor)
{
    uint32_t i = 0;
    
    comm_out(macCMD_SetPixel);  // 写内存开始
    
    for(i = 0; i < ulAmout_Point; i++)
        GC9B71_Write_Data(usColor);
}

/**
 * @brief  将GC9B71显示区域某一区域填充为某种颜色
 * @param  usX: 窗口的起始X坐标
 * @param  usY: 窗口的起始Y坐标
 * @param  usWidth: 窗口的宽度
 * @param  usHeight: 窗口的高度
 * @param  usColor: 填充颜色
 * @retval 无
 */
void GC9B71_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint32_t usColor)
{
    GC9B71_OpenWindow(usX, usY, usWidth, usHeight);
    GC9B71_FillColor(usWidth * usHeight, usColor);		
}

/**
 * @brief  在GC9B71显示区域某一点绘制像素点
 * @param  usX: 该点的X坐标
 * @param  usY: 该点的Y坐标
 * @param  usColor: 将要绘制的像素颜色
 * @retval 无
 */
void GC9B71_SetPointPixel(uint16_t usX, uint16_t usY, uint16_t usColor)	
{	
    if((usX < macGC9B71_DispWindow_COLUMN) && (usY < macGC9B71_DispWindow_PAGE))
    {
        GC9B71_SetCursor(usX, usY);
        GC9B71_FillColor(1, usColor);
    }
}

/**
 * @brief  读取GC9B71 GRAM中的一个像素数据
 * @param  无
 * @retval 像素数据
 * @note   当前SPI实现可能不支持读取
 */
static uint16_t GC9B71_Read_PixelData(void)	
{	
    uint16_t usR = 0, usG = 0, usB = 0;
    
    comm_out(0x2E);  // 读命令
    usR = GC9B71_Read_Data();  // FIRST READ OUT DUMMY DATA
    usR = GC9B71_Read_Data();  // READ OUT RED DATA
    usB = GC9B71_Read_Data();  // READ OUT BLUE DATA
    usG = GC9B71_Read_Data();  // READ OUT GREEN DATA
    
    return(((usR >> 11) << 11) | ((usG >> 10) << 5) | (usB >> 11));
}

/**
 * @brief  读取GC9B71显示区域某一坐标的像素颜色
 * @param  usX: 该点的X坐标
 * @param  usY: 该点的Y坐标
 * @retval 像素颜色
 */
uint16_t GC9B71_GetPointPixel(uint16_t usX, uint16_t usY)
{ 
    uint16_t usPixelData;
    
    GC9B71_SetCursor(usX, usY);
    usPixelData = GC9B71_Read_PixelData();
    
    return usPixelData;
}

/**
 * @brief  显示灰度渐变图案
 * @param  无
 * @retval 无
 */
void disp_gray(void)
{
    int ii, jj, kk;
    uint32_t *p;
    uint32_t xsNum, xeNum, xxNum;
    uint32_t cc;
    
    xxNum = macGC9B71_Default_Max_Width / 32;
    xsNum = (macGC9B71_Default_Max_Width - xxNum * 30) / 2; 		
    xeNum = macGC9B71_Default_Max_Width - xxNum * 30 - xsNum;
    
    GC9B71_OpenWindow(0, 0, 80, 320);    
    comm_out(macCMD_SetPixel);	 
    
    for(ii = 0; ii < macGC9B71_Default_Max_Heigth; ii++)		
    {
        cc = 0x00000000;
        for(kk = 0; kk < xsNum; kk++)
            GC9B71_Write_Data(cc);
        
        for(jj = 1; jj < 31; jj++)
        {
            cc = 0x00080808 * jj;
            for(kk = 0; kk < xxNum; kk++)
                GC9B71_Write_Data(cc);	
        }
        
        cc = 0x00080808 * 31;
        for(kk = 0; kk < xeNum; kk++)
            GC9B71_Write_Data(cc);
    }
}

/**
 * @brief  显示边框
 * @param  无
 * @retval 无
 */
void DispFrame(void)
{
    unsigned int i, j;
    
    GC9B71_OpenWindow(0, 0, 200, 480);
    comm_out(macCMD_SetPixel);
    
    for(i = 0; i < macGC9B71_Default_Max_Heigth; i++)
    { 
        for(j = 0; j < macGC9B71_Default_Max_Width; j++)
        {
            if(i == 0 || i == (macGC9B71_Default_Max_Heigth - 1) || 
               j == 0 || j == (macGC9B71_Default_Max_Width - 1))
            {
                GC9B71_Write_Data(0xffffff);  // 边框白色
            }
            else
            {
                GC9B71_Write_Data(0x000000);  // 内部黑色
            }
        }
    }
    comm_out(macCMD_SetPixel);
}

// /**
//  * @brief  显示图片
//  * @param  无
//  * @retval 无
//  */
// void pic_show(void)
// {
//     uint32_t i, j, k; 
//     GC9B71_OpenWindow(0, 0, 96, 96);  
//     comm_out(macCMD_SetPixel);	 
//     k = 0;
    
//     for(i = 0; i < macGC9B71_Default_Max_Width; i++)		
//     {		
//         for(j = 0; j < macGC9B71_Default_Max_Heigth; j++)			
//         {
//             GC9B71_Write_Data(show[k+2] << 16 | show[k+1] << 8 | show[k]);
//             k = k + 3;   
//         }		
//     }
// }

/**
 * @brief  使用Bresenham算法绘制线段
 * @param  usX1: 线段端点1的X坐标
 * @param  usY1: 线段端点1的Y坐标
 * @param  usX2: 线段端点2的X坐标
 * @param  usY2: 线段端点2的Y坐标
 * @param  usColor: 线段的颜色
 * @retval 无
 */
void GC9B71_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2, uint16_t usColor)
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
        GC9B71_SetPointPixel(usX_Current, usY_Current, usColor);
        
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
 * @param  usX_Start: 矩形起始X坐标
 * @param  usY_Start: 矩形起始Y坐标
 * @param  usWidth: 矩形宽度
 * @param  usHeight: 矩形高度
 * @param  usColor: 矩形颜色
 * @param  ucFilled: 是否填充
 *   @arg 0: 空心矩形
 *   @arg 1: 实心矩形
 * @retval 无
 */
void GC9B71_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, 
                          uint16_t usHeight, uint16_t usColor, uint8_t ucFilled)
{
    if(ucFilled)
        GC9B71_Clear(usX_Start, usY_Start, usWidth, usHeight, usColor);
    else
    {
        GC9B71_DrawLine(usX_Start, usY_Start, usX_Start + usWidth - 1, usY_Start, usColor);
        GC9B71_DrawLine(usX_Start, usY_Start + usHeight - 1, usX_Start + usWidth - 1, usY_Start + usHeight - 1, usColor);
        GC9B71_DrawLine(usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1, usColor);
        GC9B71_DrawLine(usX_Start + usWidth - 1, usY_Start, usX_Start + usWidth - 1, usY_Start + usHeight - 1, usColor);		
    }
}

/**
 * @brief  使用Bresenham算法绘制圆
 * @param  usX_Center: 圆心X坐标
 * @param  usY_Center: 圆心Y坐标
 * @param  usRadius: 圆的半径
 * @param  usColor: 圆的颜色
 * @param  ucFilled: 是否填充
 *   @arg 0: 空心圆
 *   @arg 1: 实心圆
 * @retval 无
 */
void GC9B71_DrawCircle(uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, 
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
                GC9B71_SetPointPixel(usX_Center + sCurrentX, usY_Center + sCountY, usColor);
                GC9B71_SetPointPixel(usX_Center - sCurrentX, usY_Center + sCountY, usColor);
                GC9B71_SetPointPixel(usX_Center - sCountY,   usY_Center + sCurrentX, usColor);
                GC9B71_SetPointPixel(usX_Center - sCountY,   usY_Center - sCurrentX, usColor);
                GC9B71_SetPointPixel(usX_Center - sCurrentX, usY_Center - sCountY, usColor);
                GC9B71_SetPointPixel(usX_Center + sCurrentX, usY_Center - sCountY, usColor);
                GC9B71_SetPointPixel(usX_Center + sCountY,   usY_Center - sCurrentX, usColor);
                GC9B71_SetPointPixel(usX_Center + sCountY,   usY_Center + sCurrentX, usColor);			
            }
        else
        {          
            GC9B71_SetPointPixel(usX_Center + sCurrentX, usY_Center + sCurrentY, usColor);
            GC9B71_SetPointPixel(usX_Center - sCurrentX, usY_Center + sCurrentY, usColor);
            GC9B71_SetPointPixel(usX_Center - sCurrentY, usY_Center + sCurrentX, usColor);
            GC9B71_SetPointPixel(usX_Center - sCurrentY, usY_Center - sCurrentX, usColor);
            GC9B71_SetPointPixel(usX_Center - sCurrentX, usY_Center - sCurrentY, usColor);
            GC9B71_SetPointPixel(usX_Center + sCurrentX, usY_Center - sCurrentY, usColor);
            GC9B71_SetPointPixel(usX_Center + sCurrentY, usY_Center - sCurrentX, usColor);
            GC9B71_SetPointPixel(usX_Center + sCurrentY, usY_Center + sCurrentX, usColor);
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
 * @param  usX: 字符显示起始X坐标
 * @param  usY: 字符显示起始Y坐标
 * @param  cChar: 需要显示的英文字符
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void GC9B71_DispChar_EN(uint16_t usX, uint16_t usY, const char cChar, 
                        uint16_t usColor_Background, uint16_t usColor_Foreground)
{
    uint8_t ucTemp, ucRelativePositon, ucPage, ucColumn;
    
    ucRelativePositon = cChar - ' ';
    
    GC9B71_OpenWindow(usX, usY, macWIDTH_EN_CHAR, macHEIGHT_EN_CHAR);
    comm_out(macCMD_SetPixel);	
	
    for(ucPage = 0; ucPage < macHEIGHT_EN_CHAR; ucPage++)
    {
        ucTemp = ucAscii_1608[ucRelativePositon][ucPage];
        
        for(ucColumn = 0; ucColumn < macWIDTH_EN_CHAR; ucColumn++)
        {
            if(ucTemp & 0x01)
                GC9B71_Write_Data(usColor_Foreground);
            else
                GC9B71_Write_Data(usColor_Background);
            
            ucTemp >>= 1;		
        }
    }
}

/**
 * @brief  显示英文字符串
 * @param  usX: 字符串显示起始X坐标
 * @param  usY: 字符串显示起始Y坐标
 * @param  pStr: 字符串首地址
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void GC9B71_DispString_EN(uint16_t usX, uint16_t usY, const char *pStr, 
                          uint16_t usColor_Background, uint16_t usColor_Foreground)
{
    while(*pStr != '\0')
    {
        if((usX - macGC9B71_DispWindow_X_Star + macWIDTH_EN_CHAR) > macGC9B71_DispWindow_COLUMN)
        {
            usX = macGC9B71_DispWindow_X_Star;
            usY += macHEIGHT_EN_CHAR;
        }
        
        if((usY - macGC9B71_DispWindow_Y_Star + macHEIGHT_EN_CHAR) > macGC9B71_DispWindow_PAGE)
        {
            usX = macGC9B71_DispWindow_X_Star;
            usY = macGC9B71_DispWindow_Y_Star;
        }
        
        GC9B71_DispChar_EN(usX, usY, *pStr, usColor_Background, usColor_Foreground);
        
        pStr++;
        usX += macWIDTH_EN_CHAR;
    }
}

/**
 * @brief  显示中文字符
 * @param  usX: 字符显示起始X坐标
 * @param  usY: 字符显示起始Y坐标
 * @param  usChar: 中文字符内码
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */ 
void GC9B71_DispChar_CH(uint16_t usX, uint16_t usY, uint16_t usChar, 
                        uint16_t usColor_Background, uint16_t usColor_Foreground)
{
    uint8_t ucPage, ucColumn;
    uint8_t ucBuffer[32];	
    uint16_t usTemp; 	

    GC9B71_OpenWindow(usX, usY, macWIDTH_CH_CHAR, macHEIGHT_CH_CHAR);
    comm_out(macCMD_SetPixel);
       
    macGetGBKCode(ucBuffer, usChar);
    
    for(ucPage = 0; ucPage < macHEIGHT_CH_CHAR; ucPage++)
    {
        usTemp = ucBuffer[ucPage * 2];
        usTemp = (usTemp << 8);
        usTemp |= ucBuffer[ucPage * 2 + 1];
        
        for(ucColumn = 0; ucColumn < macWIDTH_CH_CHAR; ucColumn++)
        {			
            if(usTemp & (0x01 << 15))
                GC9B71_Write_Data(usColor_Foreground);
            else
                GC9B71_Write_Data(usColor_Background);
            
            usTemp <<= 1;
        }
    }
}

/**
 * @brief  显示中文字符串
 * @param  usX: 字符串显示起始X坐标
 * @param  usY: 字符串显示起始Y坐标
 * @param  pStr: 字符串首地址
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void GC9B71_DispString_CH(uint16_t usX, uint16_t usY, const uint8_t *pStr, 
                          uint16_t usColor_Background, uint16_t usColor_Foreground)
{	
    uint16_t usCh;
    
    while(*pStr != '\0')
    {		
        if((usX - macGC9B71_DispWindow_X_Star + macWIDTH_CH_CHAR) > macGC9B71_DispWindow_COLUMN)
        {
            usX = macGC9B71_DispWindow_X_Star;
            usY += macHEIGHT_CH_CHAR;
        }
        
        if((usY - macGC9B71_DispWindow_Y_Star + macHEIGHT_CH_CHAR) > macGC9B71_DispWindow_PAGE)
        {
            usX = macGC9B71_DispWindow_X_Star;
            usY = macGC9B71_DispWindow_Y_Star;
        }	
        
        usCh = *(uint16_t *)pStr;	
        usCh = (usCh << 8) + (usCh >> 8);  // 大小端转换

        GC9B71_DispChar_CH(usX, usY, usCh, usColor_Background, usColor_Foreground);
        
        usX += macWIDTH_CH_CHAR;
        pStr += 2;  // 一个汉字占两个字节
    }	   
}

/**
 * @brief  显示中英文字符串（自动识别）
 * @param  usX: 字符串显示起始X坐标
 * @param  usY: 字符串显示起始Y坐标
 * @param  pStr: 字符串首地址
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void GC9B71_DispString_EN_CH(uint16_t usX, uint16_t usY, const uint8_t *pStr, 
                             uint16_t usColor_Background, uint16_t usColor_Foreground)
{
    uint16_t usCh;
    
    while(*pStr != '\0')
    {
        if(*pStr <= 126)  // 英文字符
        {
            if((usX - macGC9B71_DispWindow_X_Star + macWIDTH_EN_CHAR) > macGC9B71_DispWindow_COLUMN)
            {
                usX = macGC9B71_DispWindow_X_Star;
                usY += macHEIGHT_EN_CHAR;
            }
            
            if((usY - macGC9B71_DispWindow_Y_Star + macHEIGHT_EN_CHAR) > macGC9B71_DispWindow_PAGE)
            {
                usX = macGC9B71_DispWindow_X_Star;
                usY = macGC9B71_DispWindow_Y_Star;
            }			
        
            GC9B71_DispChar_EN(usX, usY, *pStr, usColor_Background, usColor_Foreground);
            
            usX += macWIDTH_EN_CHAR;
            pStr++;
        }
        else  // 中文字符
        {
            if((usX - macGC9B71_DispWindow_X_Star + macWIDTH_CH_CHAR) > macGC9B71_DispWindow_COLUMN)
            {
                usX = macGC9B71_DispWindow_X_Star;
                usY += macHEIGHT_CH_CHAR;
            }
            
            if((usY - macGC9B71_DispWindow_Y_Star + macHEIGHT_CH_CHAR) > macGC9B71_DispWindow_PAGE)
            {
                usX = macGC9B71_DispWindow_X_Star;
                usY = macGC9B71_DispWindow_Y_Star;
            }	
            
            usCh = *(uint16_t *)pStr;	
            usCh = (usCh << 8) + (usCh >> 8);

            GC9B71_DispChar_CH(usX, usY, usCh, usColor_Background, usColor_Foreground);
            
            usX += macWIDTH_CH_CHAR;
            pStr += 2;
        }
    }
} 
#endif
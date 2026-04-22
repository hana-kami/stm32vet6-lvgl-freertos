#include "bsp_ili9341_lcd.h"
#include "ascii.h"	
//#include "pic1.h"	


static void                   ILI9341_Delay               ( __IO uint32_t nCount );
static void                   ILI9341_GPIO_Config         ( void );
static void                   ILI9341_FSMC_Config         ( void );
static void                   ILI9341_REG_Config          ( void );
static void                   ILI9341_SetCursor           ( uint16_t usX, uint16_t usY );
static __inline void          ILI9341_FillColor           ( uint32_t ulAmout_Point, uint16_t usColor );
static uint16_t               ILI9341_Read_PixelData      ( void );



/**
 * @brief  输出命令到ILI9341
 * @param  usCmd : 要输出的命令值
 * @retval 无
 */
void comm_out ( uint16_t usCmd )
{
    * ( __IO uint16_t * ) ( macFSMC_Addr_ILI9341_CMD ) = usCmd;
}

/**
 * @brief  输出数据到ILI9341
 * @param  usData : 要输出的数据值
 * @retval 无
 */
void data_out ( uint16_t usData )
{
    * ( __IO uint16_t * ) ( macFSMC_Addr_ILI9341_DATA ) = usData;
}

/**
  * @brief  向ILI9341写命令
  * @param  usCmd :要写的命令（寄存器地址）
  * @retval 无
  */	
/*
__inline void ILI9341_Write_Cmd ( uint16_t usCmd )
{
    * ( __IO uint16_t * ) ( macFSMC_Addr_ILI9341_CMD ) = usCmd>>8;
    * ( __IO uint16_t * ) ( macFSMC_Addr_ILI9341_CMD ) = usCmd;
}
*/

/**
  * @brief  向ILI9341写数据
  * @param  usData :要写的数据
  * @retval 无
  */	
__inline void ILI9341_Write_Data ( uint16_t usData )
{
    * ( __IO uint16_t * ) ( macFSMC_Addr_ILI9341_DATA ) = usData>>8;
    * ( __IO uint16_t * ) ( macFSMC_Addr_ILI9341_DATA ) = usData;
}

/**
  * @brief  从ILI9341读取数据
  * @param  无
  * @retval 读取到的数据
  */	
__inline uint16_t ILI9341_Read_Data ( void )
{
    return ( * ( __IO uint16_t * ) ( macFSMC_Addr_ILI9341_DATA ) );
}

/**
  * @brief  实现 ILI9341 延时函数
  * @param  nCount 延时计数值
  * @retval 无
  */	
static void ILI9341_Delay ( __IO uint32_t nCount )
{
    for ( ; nCount != 0; nCount -- );
}

/**
  * @brief  初始化ILI9341的IO引脚
  * @param  无
  * @retval 无
  */
static void ILI9341_GPIO_Config ( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 使能各IO时钟 */
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_AFIO, ENABLE );
    
    /* 使能FSMC对应的引脚时钟 */
    RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE );
        
    /* 配置FSMC对应数据引脚, FSMC-D0~D15: PD 14 15 0 1, PE 7 8 9 10 11 12 13 14 15, PD 8 9 10 */	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_8 | GPIO_Pin_9 | 
                                  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init ( GPIOD, & GPIO_InitStructure );
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                  GPIO_Pin_15;
    GPIO_Init ( GPIOE,  & GPIO_InitStructure ); 
    
    /* 配置FSMC对应的控制引脚
     * PD4-FSMC_NOE   :LCD-RD
     * PD5-FSMC_NWE   :LCD-WR
     * PD7-FSMC_NE1   :LCD-CS
     * PD11-FSMC_A16  :LCD-DC
     */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
    GPIO_Init (GPIOD, & GPIO_InitStructure );
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
    GPIO_Init (GPIOD, & GPIO_InitStructure );
    
    macILI9341_CS_APBxClock_FUN ( macILI9341_CS_CLK, ENABLE );
    GPIO_InitStructure.GPIO_Pin = macILI9341_CS_PIN; 
    GPIO_Init ( macILI9341_CS_PORT, & GPIO_InitStructure );  
    
    macILI9341_DC_APBxClock_FUN ( macILI9341_DC_CLK, ENABLE );
    GPIO_InitStructure.GPIO_Pin = macILI9341_DC_PIN; 
    GPIO_Init ( macILI9341_DC_PORT, & GPIO_InitStructure );
    
    /* 配置LCD复位RST控制引脚 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    macILI9341_RST_APBxClock_FUN ( macILI9341_RST_CLK, ENABLE );
    GPIO_InitStructure.GPIO_Pin = macILI9341_RST_PIN; 
    GPIO_Init ( macILI9341_RST_PORT, & GPIO_InitStructure );
    
    /* 配置LCD背光控制引脚BK */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    
    macILI9341_BK_APBxClock_FUN ( macILI9341_BK_CLK, ENABLE );
    GPIO_InitStructure.GPIO_Pin = macILI9341_BK_PIN; 
    GPIO_Init ( macILI9341_BK_PORT, & GPIO_InitStructure );
    
    // IO口初始状态初始化
    GPIO_SetBits ( macILI9341_RST_PORT, macILI9341_RST_PIN );	
    GPIO_SetBits ( macILI9341_CS_PORT, macILI9341_CS_PIN );
    GPIO_SetBits ( macILI9341_DC_PORT, macILI9341_DC_PIN );
}

/**
  * @brief  LCD FSMC 模式配置
  * @param  无
  * @retval 无
  */
static void ILI9341_FSMC_Config ( void )
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p; 
    
    /* 使能FSMC时钟 */
    RCC_AHBPeriphClockCmd ( RCC_AHBPeriph_FSMC, ENABLE );

    p.FSMC_AddressSetupTime      = 0x02;	 // 地址建立时间
    p.FSMC_AddressHoldTime       = 0x00;	 // 地址保持时间
    p.FSMC_DataSetupTime         = 0x05;	 // 数据建立时间
    p.FSMC_BusTurnAroundDuration = 0x00;
    p.FSMC_CLKDivision           = 0x00;
    p.FSMC_DataLatency           = 0x00;
    p.FSMC_AccessMode            = FSMC_AccessMode_B;	 // 模式B兼容ILI9341
    
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
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = & p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = & p;  
    
    FSMC_NORSRAMInit ( & FSMC_NORSRAMInitStructure ); 
    
    /* 使能 FSMC_Bank1_NORSRAM4 */
    FSMC_NORSRAMCmd ( macFSMC_Bank1_NORSRAMx, ENABLE );  
}

/**
 * @brief  初始化ILI9341寄存器
 * @param  无
 * @retval 无
 */
static void ILI9341_REG_Config ( void )
{
#if 1		
    comm_out(0xfe);
    comm_out(0xfe);
    comm_out(0xef);
    comm_out(0x36);
    data_out(0x48); 
    comm_out(0x3a);
    data_out(0x05);
        
    comm_out(0x86);	
    data_out(0x98);
    comm_out(0x89);	
    data_out(0x03);
    comm_out(0x8b);	
    data_out(0x80);	
    comm_out(0x8d);	
    data_out(0x33);
    comm_out(0x8e);	 
    data_out(0x0f);

    comm_out(0xe8);
    data_out(0x12);
    data_out(0x00);
    
    comm_out(0xc3);	
    data_out(0x1d);
    comm_out(0xc4);	
    data_out(0x1d);
    comm_out(0xc9);	
    data_out(0x0f);
    
    comm_out(0xff);
    data_out(0x62);

    comm_out(0x99);	
    data_out(0x3e);
    comm_out(0x9d);	
    data_out(0x4b);
    comm_out(0x98);	
    data_out(0x3e);
    comm_out(0x9c);	
    data_out(0x4b);

    comm_out(0xF0);
    data_out(0x49);
    data_out(0x0b);
    data_out(0x09);
    data_out(0x08);
    data_out(0x06);
    data_out(0x2e);

    comm_out(0xF2);
    data_out(0x49);
    data_out(0x0b);
    data_out(0x09);
    data_out(0x08);
    data_out(0x06);
    data_out(0x2e);

    comm_out(0xF1);
    data_out(0x45);
    data_out(0x92);
    data_out(0x93);
    data_out(0x2b);
    data_out(0x31);
    data_out(0x6F);

    comm_out(0xF3);
    data_out(0x45);
    data_out(0x92);
    data_out(0x93);
    data_out(0x2b);
    data_out(0x31);
    data_out(0x6F);

    comm_out(0x35);
    data_out(0x00);

    comm_out(0x11);
    ILI9341_Delay(120);
    comm_out(0x29);
    comm_out(0x2c);
#endif
}

/**
 * @brief  ILI9341初始化函数，使用LCD前必须调用此函数
 * @param  无
 * @retval 无
 */
void ILI9341_Init ( void )
{
    ILI9341_GPIO_Config ();
    ILI9341_FSMC_Config ();
    
    //ILI9341_BackLed_Control ( ENABLE );      // 打开LCD背光
    ILI9341_Rst ();
    ILI9341_REG_Config ();
}

/**
 * @brief  ILI9341背光LED控制
 * @param  enumState 参数表示是否使能背光LED
 *   该参数为下列值之一：
 *      @arg ENABLE : 使能背光LED
 *      @arg DISABLE : 禁能背光LED
 * @retval 无
 */
void ILI9341_BackLed_Control ( FunctionalState enumState )
{
    if ( enumState )
        GPIO_ResetBits ( macILI9341_BK_PORT, macILI9341_BK_PIN );
    else
        GPIO_SetBits ( macILI9341_BK_PORT, macILI9341_BK_PIN );
}

/**
 * @brief  ILI9341 硬件复位
 * @param  无
 * @retval 无
 */
void ILI9341_Rst ( void )
{	
    GPIO_SetBits ( macILI9341_RST_PORT, macILI9341_RST_PIN );		 	 
    ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 	
    GPIO_ResetBits ( macILI9341_RST_PORT, macILI9341_RST_PIN );	 // 低电平复位
    ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 					   
    GPIO_SetBits ( macILI9341_RST_PORT, macILI9341_RST_PIN );		 	 
    ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 	
    ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 
    ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 	ILI9341_Delay ( 0xAFFf<<2 ); 
}

/**
 * @brief  设置ILI9341的GRAM扫描方向
 * @param  ucOption 选择GRAM的扫描方向
 *   该参数为下列值之一：
 *      @arg 1 : 左上角->右下角
 *      @arg 2 : 右下角->左上角
 *      @arg 3 : 左下角->右上角
 *      @arg 4 : 右上角->左下角
 * @retval 无
 */
void ILI9341_GramScan ( uint8_t ucOption )
{	
    switch ( ucOption )
    {
        case 1:
            // 左上角->右下角      显示英文时使用的扫描模式 
            // ____ x(240)       // 液晶屏为参考系
            // |  
            // |	y(320)        
            comm_out ( 0x36 ); 
            data_out ( 0xC8 );   
            comm_out ( macCMD_SetCoordinateX ); 
            data_out ( 0x00 );	/* x start */	
            data_out ( 0x00 );
            data_out ( 0x00 );  /* x end */	
            data_out ( 0xef );

            comm_out ( macCMD_SetCoordinateY ); 
            data_out ( 0x00 );	/* y start */  
            data_out ( 0x00 );
            data_out ( 0x01 );	/* y end */   
            data_out ( 0x3F );
            break;
        
        case 2:
            // 右下角->左上角      显示横屏图片时使用的扫描模式 
            // |x(320)            // 液晶屏为参考系
            // |
            // |___ y(240)
            comm_out ( 0x36 ); 
            data_out ( 0x68 );	
            comm_out ( macCMD_SetCoordinateX ); 
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0xeF );	

            comm_out ( macCMD_SetCoordinateY ); 
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0x01 );
            data_out ( 0x3F );		
            break;
        
        case 3:
            // 左下角->右上角       显示BMP图片时使用的扫描模式 
            //           |x(320)   // 液晶屏为参考系
            //           |           
            // y(240) ____|
            comm_out ( 0x36 ); 
            data_out ( 0x28 );	
            comm_out ( macCMD_SetCoordinateX ); 
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0xeF );	

            comm_out ( macCMD_SetCoordinateY ); 
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0x01 );
            data_out ( 0x3F );			
            break;
        
        case 4:
            // 右上角->左下角
            // |y(320)              // 液晶屏为参考系
            // |
            // |___ x(240)			
            comm_out ( 0x36 ); 
            data_out ( 0x48 );	
            comm_out ( macCMD_SetCoordinateX ); 
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0xeF );	

            comm_out ( macCMD_SetCoordinateY ); 
            data_out ( 0x00 );
            data_out ( 0x00 );
            data_out ( 0x01 );
            data_out ( 0x3F );		
            break;
    }
    
    /* write gram start */
    comm_out ( macCMD_SetPixel );
}

/**
 * @brief  在ILI9341显示区域上打开一个窗口
 * @param  usX 在特定扫描方向下窗口的起始X坐标
 * @param  usY 在特定扫描方向下窗口的起始Y坐标
 * @param  usWidth 窗口的宽度
 * @param  usHeight 窗口的高度
 * @retval 无
 */
void ILI9341_OpenWindow ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight )
{	
    comm_out ( macCMD_SetCoordinateX ); 				// 设置X坐标 //
    data_out ( (usX+0) >> 8  );	 					// 先高8位然后低8位 //
    data_out ( (usX+0 ) & 0xff  );	 				// 设置起始点和终点 //
    data_out ( ( usX+0 + usWidth - 1) >> 8  );
    data_out ( ( usX+0 + usWidth - 1 ) & 0xff  );

    comm_out ( macCMD_SetCoordinateY ); 				// 设置Y坐标 //
    data_out ( (usY+0) >> 8  );
    data_out ( (usY+0) & 0xff  );
    data_out ( ( usY +0+ usHeight - 1 ) >> 8 );
    data_out ( ( usY +0+ usHeight - 1) & 0xff );
}

/**
 * @brief  设定ILI9341的光标位置
 * @param  usX 在特定扫描方向下光标X坐标
 * @param  usY 在特定扫描方向下光标Y坐标
 * @retval 无
 */
static void ILI9341_SetCursor ( uint16_t usX, uint16_t usY )	
{
    ILI9341_OpenWindow ( usX, usY, 1, 1 );
}

/**
 * @brief  向ILI9341显示区域填充某种颜色的像素点
 * @param  ulAmout_Point 需要填充颜色的像素点的数目
 * @param  usColor 填充颜色
 * @retval 无
 */
static __inline void ILI9341_FillColor ( uint32_t ulAmout_Point, uint16_t usColor )
{
    uint32_t i = 0;
    
    /* memory write */
    comm_out ( macCMD_SetPixel );	
        
    for ( i = 0; i < ulAmout_Point; i ++ )
        ILI9341_Write_Data ( usColor );
}

/**
 * @brief  将ILI9341显示区域某一区域填充为某种颜色
 * @param  usX 在特定扫描方向下窗口的起始X坐标
 * @param  usY 在特定扫描方向下窗口的起始Y坐标
 * @param  usWidth 窗口的宽度
 * @param  usHeight 窗口的高度
 * @param  usColor 填充颜色
 * @retval 无
 */
void ILI9341_Clear ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint16_t usColor )
{
    ILI9341_OpenWindow ( usX, usY, usWidth, usHeight );
    ILI9341_FillColor ( usWidth * usHeight, usColor );		
}

/**
 * @brief  在ILI9341显示区域某一点绘制像素点
 * @param  usX 在特定扫描方向下该点的X坐标
 * @param  usY 在特定扫描方向下该点的Y坐标
 * @param  usColor 将要绘制的像素颜色
 * @retval 无
 */
void ILI9341_SetPointPixel ( uint16_t usX, uint16_t usY, uint16_t usColor )	
{	
    if ( ( usX < macILI9341_DispWindow_COLUMN ) && ( usY < macILI9341_DispWindow_PAGE ) )
    {
        ILI9341_SetCursor ( usX, usY );
        ILI9341_FillColor ( 1, usColor );
    }
}

/**
 * @brief  读取ILI9341 GRAM中的一个像素数据
 * @param  无
 * @retval 像素数据
 */
static uint16_t ILI9341_Read_PixelData ( void )	
{	
    uint16_t usR=0, usG=0, usB=0 ;
    
    comm_out ( 0x2E );   /* 读命令 */
    usR = ILI9341_Read_Data (); 	/* FIRST READ OUT DUMMY DATA */
    usR = ILI9341_Read_Data ();  	/* READ OUT RED DATA  */
    usB = ILI9341_Read_Data ();  	/* READ OUT BLUE DATA */
    usG = ILI9341_Read_Data ();  	/* READ OUT GREEN DATA */	
    
    return ( ( ( usR >> 11 ) << 11 ) | ( ( usG >> 10 ) << 5 ) | ( usB >> 11 ) );
}

/**
 * @brief  读取 ILI9341 显示区域某一坐标的像素颜色
 * @param  usX 在特定扫描方向下该点的X坐标
 * @param  usY 在特定扫描方向下该点的Y坐标
 * @retval 像素颜色
 */
uint16_t ILI9341_GetPointPixel ( uint16_t usX, uint16_t usY )
{ 
    uint16_t usPixelData;
    
    ILI9341_SetCursor ( usX, usY );
    usPixelData = ILI9341_Read_PixelData ();
    
    return usPixelData;
}

/**
 * @brief  在 ILI9341 显示区域使用 Bresenham 算法绘制线段
 * @param  usX1 在特定扫描方向下线段的一个端点X坐标
 * @param  usY1 在特定扫描方向下线段的一个端点Y坐标
 * @param  usX2 在特定扫描方向下线段的另一个端点X坐标
 * @param  usY2 在特定扫描方向下线段的另一个端点Y坐标
 * @param  usColor 线段的颜色
 * @retval 无
 */
void ILI9341_DrawLine ( uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2, uint16_t usColor )
{
    uint16_t us; 
    uint16_t usX_Current, usY_Current;
    
    int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
    int32_t lIncrease_X, lIncrease_Y; 	
    
    lDelta_X = usX2 - usX1; // 计算坐标增量 
    lDelta_Y = usY2 - usY1; 
    
    usX_Current = usX1; 
    usY_Current = usY1; 
    
    if ( lDelta_X > 0 ) 
        lIncrease_X = 1; // 设置单步方向 
    else if ( lDelta_X == 0 ) 
        lIncrease_X = 0; // 垂直线 
    else 
    { 
        lIncrease_X = -1;
        lDelta_X = - lDelta_X;
    } 
    
    if ( lDelta_Y > 0 )
        lIncrease_Y = 1; 
    else if ( lDelta_Y == 0 )
        lIncrease_Y = 0; // 水平线 
    else 
    {
        lIncrease_Y = -1;
        lDelta_Y = - lDelta_Y;
    } 
    
    if ( lDelta_X > lDelta_Y )
        lDistance = lDelta_X; // 选取基本增量方向 
    else 
        lDistance = lDelta_Y; 

    for ( us = 0; us <= lDistance + 1; us ++ ) // 画线输出 
    {  
        ILI9341_SetPointPixel ( usX_Current, usY_Current, usColor ); // 画点
        
        lError_X += lDelta_X ; 
        lError_Y += lDelta_Y ; 
        
        if ( lError_X > lDistance ) 
        { 
            lError_X -= lDistance; 
            usX_Current += lIncrease_X; 
        }  
        
        if ( lError_Y > lDistance ) 
        { 
            lError_Y -= lDistance; 
            usY_Current += lIncrease_Y; 
        } 
    }  
}

/**
 * @brief  在 ILI9341 显示区域上画一个矩形
 * @param  usX_Start 在特定扫描方向下矩形的起始X坐标
 * @param  usY_Start 在特定扫描方向下矩形的起始Y坐标
 * @param  usWidth 矩形的宽度（单位：像素）
 * @param  usHeight 矩形的高度（单位：像素）
 * @param  usColor 矩形的颜色
 * @param  ucFilled 选择是否填充矩形
 *   该参数为下列值之一：
 *      @arg 0 : 空心矩形
 *      @arg 1 : 实心矩形
 * @retval 无
 */
void ILI9341_DrawRectangle ( uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint16_t usColor, uint8_t ucFilled )
{
    if ( ucFilled )
        ILI9341_Clear ( usX_Start, usY_Start, usWidth, usHeight, usColor);
    else
    {
        ILI9341_DrawLine ( usX_Start, usY_Start, usX_Start + usWidth - 1, usY_Start, usColor );
        ILI9341_DrawLine ( usX_Start, usY_Start + usHeight - 1, usX_Start + usWidth - 1, usY_Start + usHeight - 1, usColor );
        ILI9341_DrawLine ( usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1, usColor );
        ILI9341_DrawLine ( usX_Start + usWidth - 1, usY_Start, usX_Start + usWidth - 1, usY_Start + usHeight - 1, usColor );		
    }
}

/**
 * @brief  在 ILI9341 显示区域使用 Bresenham 算法绘制圆
 * @param  usX_Center 在特定扫描方向下圆心的X坐标
 * @param  usY_Center 在特定扫描方向下圆心的Y坐标
 * @param  usRadius 圆的半径（单位：像素）
 * @param  usColor 圆的颜色
 * @param  ucFilled 选择是否填充圆
 *   该参数为下列值之一：
 *      @arg 0 : 空心圆
 *      @arg 1 : 实心圆
 * @retval 无
 */
void ILI9341_DrawCircle ( uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint16_t usColor, uint8_t ucFilled )
{
    int16_t sCurrentX, sCurrentY;
    int16_t sError;
    
    sCurrentX = 0; sCurrentY = usRadius;	  
    sError = 3 - ( usRadius << 1 );     // 判断下一个点位置的标志
    
    while ( sCurrentX <= sCurrentY )
    {
        int16_t sCountY;
        
        if ( ucFilled ) 			
            for ( sCountY = sCurrentX; sCountY <= sCurrentY; sCountY ++ ) 
            {                      
                ILI9341_SetPointPixel ( usX_Center + sCurrentX, usY_Center + sCountY,   usColor );           // 1点象限 
                ILI9341_SetPointPixel ( usX_Center - sCurrentX, usY_Center + sCountY,   usColor );           // 2       
                ILI9341_SetPointPixel ( usX_Center - sCountY,   usY_Center + sCurrentX, usColor );           // 3
                ILI9341_SetPointPixel ( usX_Center - sCountY,   usY_Center - sCurrentX, usColor );           // 4
                ILI9341_SetPointPixel ( usX_Center - sCurrentX, usY_Center - sCountY,   usColor );           // 5    
                ILI9341_SetPointPixel ( usX_Center + sCurrentX, usY_Center - sCountY,   usColor );           // 6
                ILI9341_SetPointPixel ( usX_Center + sCountY,   usY_Center - sCurrentX, usColor );           // 7 	
                ILI9341_SetPointPixel ( usX_Center + sCountY,   usY_Center + sCurrentX, usColor );           // 0				
            }
        else
        {          
            ILI9341_SetPointPixel ( usX_Center + sCurrentX, usY_Center + sCurrentY, usColor );             // 1点象限
            ILI9341_SetPointPixel ( usX_Center - sCurrentX, usY_Center + sCurrentY, usColor );             // 2      
            ILI9341_SetPointPixel ( usX_Center - sCurrentY, usY_Center + sCurrentX, usColor );             // 3
            ILI9341_SetPointPixel ( usX_Center - sCurrentY, usY_Center - sCurrentX, usColor );             // 4
            ILI9341_SetPointPixel ( usX_Center - sCurrentX, usY_Center - sCurrentY, usColor );             // 5       
            ILI9341_SetPointPixel ( usX_Center + sCurrentX, usY_Center - sCurrentY, usColor );             // 6
            ILI9341_SetPointPixel ( usX_Center + sCurrentY, usY_Center - sCurrentX, usColor );             // 7 
            ILI9341_SetPointPixel ( usX_Center + sCurrentY, usY_Center + sCurrentX, usColor );             // 0
        }			
        
        sCurrentX ++;
        
        if ( sError < 0 ) 
            sError += 4 * sCurrentX + 6;	  
        else
        {
            sError += 10 + 4 * ( sCurrentX - sCurrentY );   
            sCurrentY --;
        } 	
    }
}

/**
 * @brief  在 ILI9341 显示区域上显示一个英文字符
 * @param  usX 在特定扫描方向下字符显示起始X坐标
 * @param  usY 在特定扫描方向下该点显示起始Y坐标
 * @param  cChar 需要显示的英文字符
 * @param  usColor_Background 英文字符的背景色
 * @param  usColor_Foreground 英文字符的前景色
 * @retval 无
 */
void ILI9341_DispChar_EN ( uint16_t usX, uint16_t usY, const char cChar, uint16_t usColor_Background, uint16_t usColor_Foreground )
{
    uint8_t ucTemp, ucRelativePositon, ucPage, ucColumn;
    
    ucRelativePositon = cChar - ' ';
    
    ILI9341_OpenWindow ( usX, usY, macWIDTH_EN_CHAR, macHEIGHT_EN_CHAR );
    comm_out ( macCMD_SetPixel );	
	
    for ( ucPage = 0; ucPage < macHEIGHT_EN_CHAR; ucPage ++ )
    {
        ucTemp = ucAscii_1608 [ ucRelativePositon ] [ ucPage ];
        
        for ( ucColumn = 0; ucColumn < macWIDTH_EN_CHAR; ucColumn ++ )
        {
            if ( ucTemp & 0x01 )
                ILI9341_Write_Data ( usColor_Foreground );
            else
                ILI9341_Write_Data ( usColor_Background );								
            
            ucTemp >>= 1;		
        } /* 写入一行 */
    } /* 全部写入 */
}

/**
 * @brief  在 ILI9341 显示区域上显示英文字符串
 * @param  usX 在特定扫描方向下字符串显示的起始X坐标
 * @param  usY 在特定扫描方向下字符串显示的起始Y坐标
 * @param  pStr 需要显示的英文字符串首地址
 * @param  usColor_Background 英文字符串的背景色
 * @param  usColor_Foreground 英文字符串的前景色
 * @retval 无
 */
void ILI9341_DispString_EN ( uint16_t usX, uint16_t usY, const char * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground )
{
    while ( * pStr != '\0' )
    {
        /* 检查是否超出右边界，若超出则换行 */
        if ( ( usX - macILI9341_DispWindow_X_Star + macWIDTH_EN_CHAR ) > macILI9341_DispWindow_COLUMN )
        {
            usX = macILI9341_DispWindow_X_Star;
            usY += macHEIGHT_EN_CHAR;
        }
        
        /* 检查是否超出下边界，若超出则回到左上角 */
        if ( ( usY - macILI9341_DispWindow_Y_Star + macHEIGHT_EN_CHAR ) > macILI9341_DispWindow_PAGE )
        {
            usX = macILI9341_DispWindow_X_Star;
            usY = macILI9341_DispWindow_Y_Star;
        }
        
        ILI9341_DispChar_EN ( usX, usY, * pStr, usColor_Background, usColor_Foreground );
        
        pStr ++;
        usX += macWIDTH_EN_CHAR;
    }
}

/**
 * @brief  在 ILI9341 显示区域上显示一个中文字符
 * @param  usX 在特定扫描方向下字符显示的起始X坐标
 * @param  usY 在特定扫描方向下字符显示的起始Y坐标
 * @param  usChar 需要显示的中文字符（汉字内码）
 * @param  usColor_Background 中文字符的背景色
 * @param  usColor_Foreground 中文字符的前景色
 * @retval 无
 */ 
void ILI9341_DispChar_CH ( uint16_t usX, uint16_t usY, uint16_t usChar, uint16_t usColor_Background, uint16_t usColor_Foreground )
{
    uint8_t ucPage, ucColumn;
    uint8_t ucBuffer [ 32 ];	
    uint16_t usTemp; 	

    ILI9341_OpenWindow ( usX, usY, macWIDTH_CH_CHAR, macHEIGHT_CH_CHAR );
    comm_out ( macCMD_SetPixel );
       
    macGetGBKCode ( ucBuffer, usChar );	// 取字模数据
    
    for ( ucPage = 0; ucPage < macHEIGHT_CH_CHAR; ucPage ++ )
    {
        /* 取出两个字节的数据，在lcd上显示一个汉字的一行 */
        usTemp = ucBuffer [ ucPage * 2 ];
        usTemp = ( usTemp << 8 );
        usTemp |= ucBuffer [ ucPage * 2 + 1 ];
        
        for ( ucColumn = 0; ucColumn < macWIDTH_CH_CHAR; ucColumn ++ )
        {			
            if ( usTemp & ( 0x01 << 15 ) )  // 高位在前 
                ILI9341_Write_Data ( usColor_Foreground );
            else
                ILI9341_Write_Data ( usColor_Background );
            
            usTemp <<= 1;
        }
    }
}

/**
 * @brief  在 ILI9341 显示区域上显示中文字符串
 * @param  usX 在特定扫描方向下字符串显示的起始X坐标
 * @param  usY 在特定扫描方向下字符串显示的起始Y坐标
 * @param  pStr 需要显示的中文字符串首地址
 * @param  usColor_Background 字符串的背景色
 * @param  usColor_Foreground 字符串的前景色
 * @retval 无
 */
void ILI9341_DispString_CH ( uint16_t usX, uint16_t usY, const uint8_t * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground )
{	
    uint16_t usCh;
    
    while( * pStr != '\0' )
    {		
        /* 检查是否超出右边界，若超出则换行 */
        if ( ( usX - macILI9341_DispWindow_X_Star + macWIDTH_CH_CHAR ) > macILI9341_DispWindow_COLUMN )
        {
            usX = macILI9341_DispWindow_X_Star;
            usY += macHEIGHT_CH_CHAR;
        }
        
        /* 检查是否超出下边界，若超出则回到左上角 */
        if ( ( usY - macILI9341_DispWindow_Y_Star + macHEIGHT_CH_CHAR ) > macILI9341_DispWindow_PAGE )
        {
            usX = macILI9341_DispWindow_X_Star;
            usY = macILI9341_DispWindow_Y_Star;
        }	
        
        usCh = * ( uint16_t * ) pStr;	
        usCh = ( usCh << 8 ) + ( usCh >> 8 );  // 高低字节交换，转为小端模式

        ILI9341_DispChar_CH ( usX, usY, usCh, usColor_Background, usColor_Foreground );
        
        usX += macWIDTH_CH_CHAR;
        pStr += 2;  // 一个汉字占两个字节 
    }	   
}

/**
 * @brief  在 ILI9341 显示区域上显示中英文字符串（自动识别中英文）
 * @param  usX 在特定扫描方向下字符串显示的起始X坐标
 * @param  usY 在特定扫描方向下字符串显示的起始Y坐标
 * @param  pStr 需要显示的字符串首地址
 * @param  usColor_Background 字符串的背景色
 * @param  usColor_Foreground 字符串的前景色
 * @retval 无
 */
void ILI9341_DispString_EN_CH ( uint16_t usX, uint16_t usY, const uint8_t * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground )
{
    uint16_t usCh;
    
    while( * pStr != '\0' )
    {
        if ( * pStr <= 126 )	           	// 英文字符（ASCII码范围）
        {
            /* 检查是否超出右边界，若超出则换行 */
            if ( ( usX - macILI9341_DispWindow_X_Star + macWIDTH_EN_CHAR ) > macILI9341_DispWindow_COLUMN )
            {
                usX = macILI9341_DispWindow_X_Star;
                usY += macHEIGHT_EN_CHAR;
            }
            
            /* 检查是否超出下边界，若超出则回到左上角 */
            if ( ( usY - macILI9341_DispWindow_Y_Star + macHEIGHT_EN_CHAR ) > macILI9341_DispWindow_PAGE )
            {
                usX = macILI9341_DispWindow_X_Star;
                usY = macILI9341_DispWindow_Y_Star;
            }			
        
            ILI9341_DispChar_EN ( usX, usY, * pStr, usColor_Background, usColor_Foreground );
            
            usX += macWIDTH_EN_CHAR;
            pStr ++;
        }
        else	                            // 中文字符
        {
            /* 检查是否超出右边界，若超出则换行 */
            if ( ( usX - macILI9341_DispWindow_X_Star + macWIDTH_CH_CHAR ) > macILI9341_DispWindow_COLUMN )
            {
                usX = macILI9341_DispWindow_X_Star;
                usY += macHEIGHT_CH_CHAR;
            }
            
            /* 检查是否超出下边界，若超出则回到左上角 */
            if ( ( usY - macILI9341_DispWindow_Y_Star + macHEIGHT_CH_CHAR ) > macILI9341_DispWindow_PAGE )
            {
                usX = macILI9341_DispWindow_X_Star;
                usY = macILI9341_DispWindow_Y_Star;
            }	
            
            usCh = * ( uint16_t * ) pStr;	
            usCh = ( usCh << 8 ) + ( usCh >> 8 );  // 高低字节交换，转为小端模式

            ILI9341_DispChar_CH ( usX, usY, usCh, usColor_Background, usColor_Foreground );
            
            usX += macWIDTH_CH_CHAR;
            pStr += 2;  // 一个汉字占两个字节 
        }
    }
} 

/*
// 显示16位图片的函数
void show_16pic()
{
    uint16_t i,j; 
    uint16_t m,temp;
    m=0;
    ILI9341_OpenWindow ( 0, 0, 128, 128 );
    comm_out ( macCMD_SetPixel );	
    for(i=0;i<128;i++)	//i=224;i<1024;i++	
    {		
        for (j=0;j<128;j++)			
        {
            temp = (show1[m+1]<<8)|(show1[m]);
            m=m+2;
            ILI9341_Write_Data ( temp );			
        }		
    }
}
*/

/**
 * @brief  颜色表数组，用于显示颜色条
 *        0:红色, 1:绿色, 2:蓝色, 3:黄色, 4:品红, 5:青色, 6:白色, 7:黑色
 */
int cb[8] =
{
    0xf800, 0x07e0, 0x001f, 0xffe0, 0xf81f, 0x07ff, 0xffff, 0x0000	
};

/**
 * @brief  显示颜色条（用于测试LCD显示效果）
 * @param  无
 * @retval 无
 */
void disp_color_table(void)
{
    int ii, jj, kk;
    ILI9341_OpenWindow ( 0, 0, 240, 320 );
    comm_out ( macCMD_SetPixel );	  
    
    for(ii = 0; ii < 320; ii++)		
    {
        for(jj = 0; jj < 7; jj++)
        {
            /* 每种颜色显示240/8=30个像素点 */
            for(kk = 0; kk < 240 / 8; kk++) 
                ILI9341_Write_Data ( cb[jj] );
        }
        
        /* 剩余部分用黑色填充 */
        for(kk = 0; kk < (240 - 30 * 7); kk++) 
            ILI9341_Write_Data(cb[7]);
    }
}

/**
 * @brief  显示9宫格图案（棋盘格效果）
 * @param  无
 * @retval 无
 */
void DISP9W(void)
{
    unsigned int i, j;
    unsigned int a, b;
    
    ILI9341_OpenWindow ( 0, 0, 240, 320); 
    comm_out ( macCMD_SetPixel );
    
    a = 320 / 5;  // 每个区域的高度
    b = 240 / 5;  // 每个区域的宽度

    for(i = 0; i < 320; i++)
    { 
        for(j = 0; j < 240; j++)
        {
            /* 判断是否在奇数行区域（第1、3、5行） */
            if(i < a || (i >= a * 2 && i < a * 3) || i >= a * 4)
            {
                /* 判断是否在奇数列区域（第1、3、5列） */
                if(j < b || (j >= b * 2 && j < b * 3) || j >= b * 4)
                    ILI9341_Write_Data(0xFFFF);  // 白色
                else
                    ILI9341_Write_Data(0x0000);  // 黑色
            }
            else  // 偶数行区域（第2、4行）
            {
                /* 判断是否在奇数列区域（第1、3、5列） */
                if(j < b || (j >= b * 2 && j < b * 3) || j >= b * 4)
                    ILI9341_Write_Data(0x0000);  // 黑色
                else
                    ILI9341_Write_Data(0xFFFF);  // 白色
            }
        }
    }
}

/**
 * @brief  显示边框（在LCD边缘绘制白色边框）
 * @param  无
 * @retval 无
 */
void side(void)
{
    int ii, jj;
    
    ILI9341_OpenWindow ( 0, 0, 240, 320 );
    comm_out ( macCMD_SetPixel );	  
    
    for(ii = 0; ii < 320; ii++)
    {
        if(ii == 0)  // 第一行
        {
            for(jj = 0; jj < 240; jj++)	 
                ILI9341_Write_Data ( 0xffff );  // 白色
        }
        else if(ii == (320 - 1))  // 最后一行
        {
            for(jj = 0; jj < 240; jj++)	 
                ILI9341_Write_Data ( 0xffff );  // 白色
        }
        else  // 中间行
        {
            for(jj = 0; jj < 240; jj++)	 
            {
                if(jj == 0 || jj == (240 - 1))  // 第一列或最后一列
                    ILI9341_Write_Data ( 0xffff );  // 白色
                else
                    ILI9341_Write_Data ( 0x0000 );  // 黑色
            }	
        }
    }
}

/**
 * @brief  RGB24格式转RGB16格式
 * @param  R 红色分量（0-255）
 * @param  G 绿色分量（0-255）
 * @param  B 蓝色分量（0-255）
 * @retval RGB565格式的颜色值
 */
#define RGB24TORGB16(R,G,B) ((unsigned short int)((((R)>>3)<<11) | (((G)>>2)<<5) | ((B)>>3)))

/**
 * @brief  显示灰度渐变图案
 * @param  无
 * @retval 无
 */
void disp_gray(void)
{
    unsigned int i, j, k;
    
    ILI9341_OpenWindow ( 0, 0, 240, 320 );
    comm_out ( macCMD_SetPixel );	

    /* 先填充不足32行的部分为黑色 */
    for(k = 0; k < (320 % 32); k++)
    {
        for(j = 0; j < 240; j++)
        {
            ILI9341_Write_Data(0x0000);
        }			
    }
    
    /* 显示32级灰度渐变 */
    for(k = 0; k < 32; k++)
    {	
        for(i = 0; i < 320 / 32; i++)
        {	
            for(j = 0; j < 240; j++)
            {
                /* R=G=B=灰度值，生成从黑到白的渐变效果 */
                ILI9341_Write_Data( RGB24TORGB16( k << 3, k << 3, k << 3 ) );
            }
        }
    }
}

/**
  * @brief  LVGL专用：批量刷新显示区域
  * @param  usX 起始X坐标
  * @param  usY 起始Y坐标  
  * @param  usWidth 宽度
  * @param  usHeight 高度
  * @param  pColors 颜色数据指针（RGB565格式）
  * @retval 无
  */
void ILI9341_LVGL_Flush(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint16_t *pColors)
{
    uint32_t ulPixelCount = usWidth * usHeight;
    uint32_t i;
    // 打开窗口
    ILI9341_OpenWindow(usX, usY, usWidth, usHeight);
    comm_out(0x2C);
    
    for(i = 0; i < ulPixelCount; i++) {
        *(__IO uint16_t *)(macFSMC_Addr_ILI9341_DATA) = pColors[i];
    }
}

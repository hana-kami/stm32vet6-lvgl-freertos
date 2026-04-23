/**
 * @file bsp_xpt2046_lcd.c
 * @brief XPT2046 触摸屏驱动（SPI接口）
 * @note 适用于4线电阻式触摸屏，使用XPT2046触摸控制器
 */

#include "bsp_xpt2046_lcd.h"
#include "bsp_lcd.h"      // 改为包含统一LCD接口
#include <stdio.h> 
#include <string.h>

/* ==================== XPT2046 相关静态函数声明 ==================== */
static void                   XPT2046_EXTI_Config                   ( void );
static void                   XPT2046_EXTI_NVIC_Config              ( void );
static void                   XPT2046_GPIO_SPI_Config               ( void );

static void                   XPT2046_DelayUS                       ( __IO uint32_t ulCount );
static void                   XPT2046_WriteCMD                      ( uint8_t ucCmd );
static uint16_t               XPT2046_ReadCMD                       ( void );
static uint16_t               XPT2046_ReadAdc                       ( uint8_t ucChannel );
static void                   XPT2046_ReadAdc_XY                    ( int16_t * sX_Ad, int16_t * sY_Ad );
static uint8_t                XPT2046_ReadAdc_Smooth_XY             ( strType_XPT2046_Coordinate * pScreenCoordinate );
static uint8_t                XPT2046_Calculate_CalibrationFactor   ( strType_XPT2046_Coordinate * pDisplayCoordinate, 
                                                                        strType_XPT2046_Coordinate * pScreenSample, 
                                                                        strType_XPT2046_Calibration * pCalibrationFactor );
static void                   LCD_DrawCross                         ( uint16_t usX, uint16_t usY );

/* ==================== XPT2046 全局变量 ==================== */
/**
 * @brief XPT2046触摸屏校准参数
 * @note 初始值为扫描方式2下的校准系数，重新校准时会更新
 *       结构体成员含义：
 *       dX_X: X坐标的X分量系数
 *       dX_Y: X坐标的Y分量系数  
 *       dX:   X坐标偏移量
 *       dY_X: Y坐标的X分量系数
 *       dY_Y: Y坐标的Y分量系数
 *       dY:   Y坐标偏移量
 */
strType_XPT2046_TouchPara strXPT2046_TouchPara = { 
    0.085958,   -0.001073,   -4.979353, 
    -0.001750,   0.065168,   -13.318824 
};

volatile uint8_t ucXPT2046_TouchFlag = 0;  /**< 触摸标志位，在中断中置位 */

/**
 * @brief  XPT2046 初始化函数
 * @param  无
 * @retval 无
 */
void XPT2046_Init ( void )
{
    XPT2046_GPIO_SPI_Config ();  // 初始化SPI引脚
    XPT2046_EXTI_Config ();      // 配置触摸中断
}

/**
 * @brief  配置XPT2046外部中断优先级
 * @param  无
 * @retval 无
 */
static void XPT2046_EXTI_NVIC_Config ( void )
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 配置中断源 */
    NVIC_InitStructure.NVIC_IRQChannel = macXPT2046_EXTI_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  配置XPT2046外部中断（下降沿触发）
 * @param  无
 * @retval 无
 */
static void XPT2046_EXTI_Config ( void )
{
    GPIO_InitTypeDef GPIO_InitStructure; 
    EXTI_InitTypeDef EXTI_InitStructure;
    
    /* 使能EXTI线和AFIO时钟 */
    RCC_APB2PeriphClockCmd ( macXPT2046_EXTI_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE );
    
    /* 配置NVIC中断优先级 */
    XPT2046_EXTI_NVIC_Config ();
    
    /* 配置EXTI线GPIO */	
    GPIO_InitStructure.GPIO_Pin = macXPT2046_EXTI_GPIO_PIN;       
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(macXPT2046_EXTI_GPIO_PORT, &GPIO_InitStructure);
    
    /* 配置EXTI线模式 */
    GPIO_EXTILineConfig(macXPT2046_EXTI_SOURCE_PORT, macXPT2046_EXTI_SOURCE_PIN); 
    EXTI_InitStructure.EXTI_Line = macXPT2046_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  // 下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    
    EXTI_Init(&EXTI_InitStructure); 
}

/**
 * @brief  配置XPT2046的软件模拟SPI引脚
 * @param  无
 * @retval 无
 */
static void XPT2046_GPIO_SPI_Config ( void ) 
{ 
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    /* 使能GPIO时钟 */
    RCC_APB2PeriphClockCmd ( macXPT2046_SPI_GPIO_CLK, ENABLE );
    
    /* 模拟SPI GPIO初始化 */          
    GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_CLK_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(macXPT2046_SPI_CLK_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_MOSI_PIN;
    GPIO_Init(macXPT2046_SPI_MOSI_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_MISO_PIN; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(macXPT2046_SPI_MISO_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_CS_PIN; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
    GPIO_Init(macXPT2046_SPI_CS_PORT, &GPIO_InitStructure); 
    
    /* 默认不选中XPT2046 */
    macXPT2046_CS_DISABLE();
}

/**
 * @brief  微秒级延时函数
 * @param  ulCount 延时计数值，约1us
 * @retval 无
 */
static void XPT2046_DelayUS ( __IO uint32_t ulCount )
{
    uint32_t i;
    
    for ( i = 0; i < ulCount; i ++ )
    {
        uint8_t uc = 12;  // 经验值，约1微秒  
        while ( uc -- );
    }
}

/**
 * @brief  向XPT2046写入命令（8位）
 * @param  ucCmd 命令字节
 *   @arg 0x90 : 选择Y+通道测量
 *   @arg 0xd0 : 选择X+通道测量
 * @retval 无
 */
static void XPT2046_WriteCMD ( uint8_t ucCmd ) 
{
    uint8_t i;
    
    macXPT2046_MOSI_0();
    macXPT2046_CLK_LOW();
    
    for ( i = 0; i < 8; i ++ ) 
    {
        /* 发送数据位（高位在前） */
        ( ( ucCmd >> ( 7 - i ) ) & 0x01 ) ? macXPT2046_MOSI_1() : macXPT2046_MOSI_0();
        
        XPT2046_DelayUS ( 5 );
        macXPT2046_CLK_HIGH();
        XPT2046_DelayUS ( 5 );
        macXPT2046_CLK_LOW();
    }
}

/**
 * @brief  从XPT2046读取数据（12位）
 * @param  无
 * @retval 读取到的12位ADC值
 */
static uint16_t XPT2046_ReadCMD ( void ) 
{
    uint8_t i;
    uint16_t usBuf = 0, usTemp;
    
    macXPT2046_MOSI_0();
    macXPT2046_CLK_HIGH();
    
    for ( i = 0; i < 12; i++ ) 
    {
        macXPT2046_CLK_LOW();    
        usTemp = macXPT2046_MISO();
        usBuf |= usTemp << ( 11 - i );
        macXPT2046_CLK_HIGH();
    }
    
    return usBuf;
}

/**
 * @brief  读取指定通道的ADC值
 * @param  ucChannel 通道选择
 *   @arg 0x90 : Y+通道
 *   @arg 0xd0 : X+通道
 * @retval 12位ADC采样值（0-4095）
 */
static uint16_t XPT2046_ReadAdc ( uint8_t ucChannel )
{
    XPT2046_WriteCMD ( ucChannel );
    return XPT2046_ReadCMD ();
}

/**
 * @brief  读取X和Y通道的ADC原始值
 * @param  sX_Ad X通道AD值存储地址
 * @param  sY_Ad Y通道AD值存储地址
 * @retval 无
 */
static void XPT2046_ReadAdc_XY ( int16_t * sX_Ad, int16_t * sY_Ad )  
{ 
    int16_t sX_Ad_Temp, sY_Ad_Temp; 
    
    sX_Ad_Temp = XPT2046_ReadAdc ( macXPT2046_CHANNEL_X );
    XPT2046_DelayUS ( 1 ); 
    sY_Ad_Temp = XPT2046_ReadAdc ( macXPT2046_CHANNEL_Y ); 
    
    *sX_Ad = sX_Ad_Temp; 
    *sY_Ad = sY_Ad_Temp; 
}

/**
 * @brief  读取触摸点ADC值并进行平滑滤波（简化版）
 * @param  pScreenCoordinate 存储滤波后的坐标
 * @retval 1:成功读取, 0:读取失败
 * @note   采样10次，去除最大最小值后取平均
 */
static uint8_t XPT2046_ReadAdc_Smooth_XY ( strType_XPT2046_Coordinate * pScreenCoordinate )
{
    uint8_t ucCount = 0, i;
    int16_t sAD_X, sAD_Y;
    int16_t sBufferArray [ 2 ] [ 10 ] = { { 0 },{ 0 } };  // X和Y各10次采样
    int32_t lX_Min, lX_Max, lY_Min, lY_Max;
    
    /* 循环采样10次 */
    do					       				
    {		  
        XPT2046_ReadAdc_XY ( & sAD_X, & sAD_Y );  
        sBufferArray [ 0 ] [ ucCount ] = sAD_X;  
        sBufferArray [ 1 ] [ ucCount ] = sAD_Y;
        ucCount ++;  
    } while ( ( macXPT2046_EXTI_Read() == macXPT2046_EXTI_ActiveLevel ) && ( ucCount < 10 ) );
    
    /* 触摸释放时清除标志位 */
    if ( macXPT2046_EXTI_Read() != macXPT2046_EXTI_ActiveLevel )
        ucXPT2046_TouchFlag = 0;
    
    /* 成功采样10次后进行滤波 */
    if ( ucCount == 10 )		 					
    {
        /* 找出X和Y的最大最小值 */
        lX_Max = lX_Min = sBufferArray [ 0 ] [ 0 ];
        lY_Max = lY_Min = sBufferArray [ 1 ] [ 0 ];       
        
        for ( i = 1; i < 10; i ++ )
        {
            if ( sBufferArray [ 0 ] [ i ] < lX_Min ) lX_Min = sBufferArray [ 0 ] [ i ];
            else if ( sBufferArray [ 0 ] [ i ] > lX_Max ) lX_Max = sBufferArray [ 0 ] [ i ];
        }
        
        for ( i = 1; i < 10; i ++ )
        {
            if ( sBufferArray [ 1 ] [ i ] < lY_Min ) lY_Min = sBufferArray [ 1 ] [ i ];
            else if ( sBufferArray [ 1 ] [ i ] > lY_Max ) lY_Max = sBufferArray [ 1 ] [ i ];
        }
        
        /* 去除最大最小值后求平均（除以8相当于右移3位）*/
        pScreenCoordinate ->x = ( sBufferArray [ 0 ] [ 0 ] + sBufferArray [ 0 ] [ 1 ] + 
                                   sBufferArray [ 0 ] [ 2 ] + sBufferArray [ 0 ] [ 3 ] + 
                                   sBufferArray [ 0 ] [ 4 ] + sBufferArray [ 0 ] [ 5 ] + 
                                   sBufferArray [ 0 ] [ 6 ] + sBufferArray [ 0 ] [ 7 ] + 
                                   sBufferArray [ 0 ] [ 8 ] + sBufferArray [ 0 ] [ 9 ] - 
                                   lX_Min - lX_Max ) >> 3;
        
        pScreenCoordinate ->y = ( sBufferArray [ 1 ] [ 0 ] + sBufferArray [ 1 ] [ 1 ] + 
                                   sBufferArray [ 1 ] [ 2 ] + sBufferArray [ 1 ] [ 3 ] + 
                                   sBufferArray [ 1 ] [ 4 ] + sBufferArray [ 1 ] [ 5 ] + 
                                   sBufferArray [ 1 ] [ 6 ] + sBufferArray [ 1 ] [ 7 ] + 
                                   sBufferArray [ 1 ] [ 8 ] + sBufferArray [ 1 ] [ 9 ] - 
                                   lY_Min - lY_Max ) >> 3; 
        
        return 1;  // 滤波成功
    }   
    
    return 0;  // 采样不足
}

/**
 * @brief  计算触摸屏校准系数（三点校准算法）
 * @param  pDisplayCoordinate LCD显示坐标（已知点）
 * @param  pScreenSample 触摸采样坐标
 * @param  pCalibrationFactor 输出校准系数
 * @retval 1:计算成功, 0:计算失败
 */
static uint8_t XPT2046_Calculate_CalibrationFactor ( strType_XPT2046_Coordinate * pDisplayCoordinate, 
                                                       strType_XPT2046_Coordinate * pScreenSample, 
                                                       strType_XPT2046_Calibration * pCalibrationFactor )
{
    uint8_t ucRet = 1;
    
    /* 计算分母 K = (X0-X2)*(Y1-Y2) - (X1-X2)*(Y0-Y2) */
    pCalibrationFactor -> Divider = ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  
                                       ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
                                     ( ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) *  
                                       ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) );
    
    if ( pCalibrationFactor -> Divider == 0 )
        ucRet = 0;
    else
    {
        /* 计算校准系数 An, Bn, Cn, Dn, En, Fn */
        pCalibrationFactor -> An = ( ( pDisplayCoordinate [ 0 ] .x - pDisplayCoordinate [ 2 ] .x ) *  
                                       ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
                                     ( ( pDisplayCoordinate [ 1 ] .x - pDisplayCoordinate [ 2 ] .x ) *  
                                       ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) );
        
        pCalibrationFactor -> Bn = ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  
                                       ( pDisplayCoordinate [ 1 ] .x - pDisplayCoordinate [ 2 ] .x ) ) - 
                                     ( ( pDisplayCoordinate [ 0 ] .x - pDisplayCoordinate [ 2 ] .x ) *  
                                       ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) );
        
        pCalibrationFactor -> Cn = ( pScreenSample [ 2 ] .x * pDisplayCoordinate [ 1 ] .x - 
                                      pScreenSample [ 1 ] .x * pDisplayCoordinate [ 2 ] .x ) * pScreenSample [ 0 ] .y +
                                    ( pScreenSample [ 0 ] .x * pDisplayCoordinate [ 2 ] .x - 
                                      pScreenSample [ 2 ] .x * pDisplayCoordinate [ 0 ] .x ) * pScreenSample [ 1 ] .y +
                                    ( pScreenSample [ 1 ] .x * pDisplayCoordinate [ 0 ] .x - 
                                      pScreenSample [ 0 ] .x * pDisplayCoordinate [ 1 ] .x ) * pScreenSample [ 2 ] .y ;
        
        pCalibrationFactor -> Dn = ( ( pDisplayCoordinate [ 0 ] .y - pDisplayCoordinate [ 2 ] .y ) *  
                                       ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
                                     ( ( pDisplayCoordinate [ 1 ] .y - pDisplayCoordinate [ 2 ] .y ) *  
                                       ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) );
        
        pCalibrationFactor -> En = ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  
                                       ( pDisplayCoordinate [ 1 ] .y - pDisplayCoordinate [ 2 ] .y ) ) - 
                                     ( ( pDisplayCoordinate [ 0 ] .y - pDisplayCoordinate [ 2 ] .y ) *  
                                       ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) );
        
        pCalibrationFactor -> Fn = ( pScreenSample [ 2 ] .x * pDisplayCoordinate [ 1 ] .y - 
                                      pScreenSample [ 1 ] .x * pDisplayCoordinate [ 2 ] .y ) * pScreenSample [ 0 ] .y +
                                    ( pScreenSample [ 0 ] .x * pDisplayCoordinate [ 2 ] .y - 
                                      pScreenSample [ 2 ] .x * pDisplayCoordinate [ 0 ] .y ) * pScreenSample [ 1 ] .y +
                                    ( pScreenSample [ 1 ] .x * pDisplayCoordinate [ 0 ] .y - 
                                      pScreenSample [ 0 ] .x * pDisplayCoordinate [ 1 ] .y ) * pScreenSample [ 2 ] .y;
    }
    
    return ucRet;
}

/**
 * @brief  在LCD上绘制校准用的十字光标
 * @param  usX 十字中心X坐标
 * @param  usY 十字中心Y坐标
 * @retval 无
 */
static void LCD_DrawCross ( uint16_t usX, uint16_t usY )
{
    LCD_ClearArea ( usX - 10, usY, 20, 1, LCD_COLOR_RED);   // 横线
    LCD_ClearArea ( usX, usY - 10, 1, 20, LCD_COLOR_RED);   // 竖线
}

/**
 * @brief  触摸屏校准函数
 * @param  无
 * @retval 1:校准成功, 0:校准失败
 */
uint8_t XPT2046_Touch_Calibrate ( void )
{
    uint8_t i;
    char cStr [ 10 ];
    uint16_t usScreenWidth, usScreenHeigth;
    uint16_t usTest_x = 0, usTest_y = 0, usGap_x = 0, usGap_y = 0;
    char * pStr = 0;
    
    strType_XPT2046_Coordinate strCrossCoordinate [ 4 ], strScreenSample [ 4 ];
    strType_XPT2046_Calibration CalibrationFactor;
    
    /* 根据扫描方向获取屏幕尺寸 */
#if ( macXPT2046_Coordinate_GramScan == 1 ) || ( macXPT2046_Coordinate_GramScan == 4 )
    usScreenWidth = LCD_GetWidth();
    usScreenHeigth = LCD_GetHeight();
#elif ( macXPT2046_Coordinate_GramScan == 2 ) || ( macXPT2046_Coordinate_GramScan == 3 )
    usScreenWidth = LCD_GetHeight();
    usScreenHeigth = LCD_GetWidth();
#endif
    
    /* 设置4个校准点的坐标 */
    strCrossCoordinate [ 0 ] .x = usScreenWidth >> 2;      // 1/4宽度
    strCrossCoordinate [ 0 ] .y = usScreenHeigth >> 2;     // 1/4高度
    
    strCrossCoordinate [ 1 ] .x = strCrossCoordinate [ 0 ] .x;
    strCrossCoordinate [ 1 ] .y = ( usScreenHeigth * 3 ) >> 2;  // 3/4高度
    
    strCrossCoordinate [ 2 ] .x = ( usScreenWidth * 3 ) >> 2;   // 3/4宽度
    strCrossCoordinate [ 2 ] .y = strCrossCoordinate [ 1 ] .y;
    
    strCrossCoordinate [ 3 ] .x = strCrossCoordinate [ 2 ] .x;
    strCrossCoordinate [ 3 ] .y = strCrossCoordinate [ 0 ] .y;		
    
    /* 设置LCD扫描方向 */
    LCD_SetScanDirection ( macXPT2046_Coordinate_GramScan );
    
    /* 依次采集4个校准点的触摸值 */
    for ( i = 0; i < 4; i ++ )
    { 
        LCD_Clear ( LCD_COLOR_BLACK );  // 清屏
        
        pStr = "Touch Calibrate ......";			
        LCD_ShowString ( ( usScreenWidth - ( strlen ( pStr ) - 7 ) * 8 ) >> 1, 
                         usScreenHeigth >> 1, 
                         pStr, LCD_COLOR_BLACK, LCD_COLOR_RED );			
        
        sprintf ( cStr, "%d", i + 1 );
        LCD_ShowString ( usScreenWidth >> 1, ( usScreenHeigth >> 1 ) - 16, 
                         cStr, LCD_COLOR_BLACK, LCD_COLOR_RED );
        
        XPT2046_DelayUS ( 100000 );  // 适当延时
        
        LCD_DrawCross ( strCrossCoordinate [ i ] .x, strCrossCoordinate [ i ] .y );  // 显示十字光标
        
        while ( !XPT2046_ReadAdc_Smooth_XY ( & strScreenSample [ i ] ) );  // 等待触摸
    }
    
    /* 计算校准系数 */
    XPT2046_Calculate_CalibrationFactor ( strCrossCoordinate, strScreenSample, & CalibrationFactor );
    
    if ( CalibrationFactor .Divider == 0 ) 
        goto Failure;
    
    /* 验证校准结果 - 使用第4个点验证 */
    usTest_x = ( ( CalibrationFactor .An * strScreenSample [ 3 ] .x ) + 
                 ( CalibrationFactor .Bn * strScreenSample [ 3 ] .y ) + 
                 CalibrationFactor .Cn ) / CalibrationFactor .Divider;
    
    usTest_y = ( ( CalibrationFactor .Dn * strScreenSample [ 3 ] .x ) + 
                 ( CalibrationFactor .En * strScreenSample [ 3 ] .y ) + 
                 CalibrationFactor .Fn ) / CalibrationFactor .Divider;
    
    /* 计算误差 */
    usGap_x = ( usTest_x > strCrossCoordinate [ 3 ] .x ) ? 
              ( usTest_x - strCrossCoordinate [ 3 ] .x ) : 
              ( strCrossCoordinate [ 3 ] .x - usTest_x );
    
    usGap_y = ( usTest_y > strCrossCoordinate [ 3 ] .y ) ? 
              ( usTest_y - strCrossCoordinate [ 3 ] .y ) : 
              ( strCrossCoordinate [ 3 ] .y - usTest_y );
    
    /* 误差超过10像素则认为校准失败 */
    if ( ( usGap_x > 10 ) || ( usGap_y > 10 ) ) 
        goto Failure;
    
    /* 保存校准系数到全局变量 */
    strXPT2046_TouchPara .dX_X = ( CalibrationFactor .An * 1.0 ) / CalibrationFactor .Divider;
    strXPT2046_TouchPara .dX_Y = ( CalibrationFactor .Bn * 1.0 ) / CalibrationFactor .Divider;
    strXPT2046_TouchPara .dX   = ( CalibrationFactor .Cn * 1.0 ) / CalibrationFactor .Divider;
    
    strXPT2046_TouchPara .dY_X = ( CalibrationFactor .Dn * 1.0 ) / CalibrationFactor .Divider;
    strXPT2046_TouchPara .dY_Y = ( CalibrationFactor .En * 1.0 ) / CalibrationFactor .Divider;
    strXPT2046_TouchPara .dY   = ( CalibrationFactor .Fn * 1.0 ) / CalibrationFactor .Divider;
    
    /* 校准成功提示 */
    LCD_Clear ( LCD_COLOR_BLACK );
    pStr = "Calibrate Succed";			
    LCD_ShowString ( ( usScreenWidth - strlen ( pStr ) * 8 ) >> 1, 
                     usScreenHeigth >> 1, 
                     pStr, LCD_COLOR_BLACK, LCD_COLOR_RED );	
    
    XPT2046_DelayUS ( 200000 );
    return 1;    
    
Failure:
    /* 校准失败提示 */
    LCD_Clear ( LCD_COLOR_BLACK ); 
    pStr = "Calibrate fail";			
    LCD_ShowString ( ( usScreenWidth - strlen ( pStr ) * 8 ) >> 1, 
                     usScreenHeigth >> 1, 
                     pStr, LCD_COLOR_BLACK, LCD_COLOR_RED );	
    
    pStr = "try again";			
    LCD_ShowString ( ( usScreenWidth - strlen ( pStr ) * 8 ) >> 1, 
                     ( usScreenHeigth >> 1 ) + 16, 
                     pStr, LCD_COLOR_BLACK, LCD_COLOR_RED );				
    
    XPT2046_DelayUS ( 1000000 );		
    return 0; 
}

/**
 * @brief  获取触摸点坐标（已校准）
 * @param  pDisplayCoordinate 输出坐标存储地址
 * @param  pTouchPara 校准参数指针
 * @retval 1:获取成功, 0:无触摸
 */
uint8_t XPT2046_Get_TouchedPoint ( strType_XPT2046_Coordinate * pDisplayCoordinate, 
                                    strType_XPT2046_TouchPara * pTouchPara )
{
    uint8_t ucRet = 1;
    strType_XPT2046_Coordinate strScreenCoordinate; 
    
    /* 读取滤波后的触摸ADC值 */
    if ( XPT2046_ReadAdc_Smooth_XY ( & strScreenCoordinate ) )
    {    
        /* 应用校准系数转换为屏幕坐标 */
        pDisplayCoordinate ->x = ( ( pTouchPara ->dX_X * strScreenCoordinate .x ) + 
                                   ( pTouchPara ->dX_Y * strScreenCoordinate .y ) + 
                                   pTouchPara ->dX );
        
        pDisplayCoordinate ->y = ( ( pTouchPara ->dY_X * strScreenCoordinate .x ) + 
                                   ( pTouchPara ->dY_Y * strScreenCoordinate .y ) + 
                                   pTouchPara ->dY );
    }
    else 
        ucRet = 0;  // 无触摸
    
    return ucRet;
}
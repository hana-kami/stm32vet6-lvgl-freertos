#ifndef      __BSP_GC9B71_LCD_H
#define      __BSP_GC9B71_LCD_H

#include "stm32f10x.h"
#include "bsp_sdfs_app.h"    // 获取SD卡字库数据

/**
 * @file bsp_gc9b71_lcd.h
 * @brief GC9B71 LCD屏幕驱动头文件（SPI接口版本）
 */

/***************************************************************************************
 * SPI引脚定义（软件模拟SPI）
 ***************************************************************************************/
#define      LCD_SDA_CLR		     GPIO_ResetBits(GPIOD, GPIO_Pin_4);	 // 数据线低电平
#define      LCD_SDA_SET		     GPIO_SetBits  (GPIOD, GPIO_Pin_4);   // 数据线高电平

#define      LCD_SCL_CLR			   GPIO_ResetBits(GPIOD, GPIO_Pin_5);	 // 时钟线低电平
#define      LCD_SCL_SET		     GPIO_SetBits  (GPIOD, GPIO_Pin_5);   // 时钟线高电平

#define      LCD_CS_CLR			   GPIO_ResetBits(GPIOE, GPIO_Pin_11);	 // 片选低电平（使能）
#define      LCD_CS_SET			   GPIO_SetBits  (GPIOE, GPIO_Pin_11);   // 片选高电平（释放）

#define      LCD_RS_CLR			   GPIO_ResetBits(GPIOD, GPIO_Pin_11);	 // RS低电平（命令模式）
#define      LCD_RS_SET			   GPIO_SetBits  (GPIOD, GPIO_Pin_11);   // RS高电平（数据模式）

/***************************************************************************************
 * 内存映射说明（原FSMC配置保留，实际使用SPI时可能不需要）
 * 2^26 = 0X0400 0000 = 64MB，每个BANK有4*64MB = 256MB
 * 64MB: FSMC_Bank1_NORSRAM1: 0X6000 0000 ~ 0X63FF FFFF
 * 64MB: FSMC_Bank1_NORSRAM2: 0X6400 0000 ~ 0X67FF FFFF
 * 64MB: FSMC_Bank1_NORSRAM3: 0X6800 0000 ~ 0X6BFF FFFF
 * 64MB: FSMC_Bank1_NORSRAM4: 0X6C00 0000 ~ 0X6FFF FFFF
 * 
 * 注：GC9B71使用SPI通信，以下FSMC地址定义仅作保留
 ****************************************************************************************/
#define      macFSMC_Addr_GC9B71_CMD         ( ( uint32_t ) 0x60000000 )	  // 命令地址（保留）
#define      macFSMC_Addr_GC9B71_DATA        ( ( uint32_t ) 0x60020000 )    // 数据地址（保留）
#define      macFSMC_Bank1_NORSRAMx           FSMC_Bank1_NORSRAM1

/***************************************************************************************
 * GC9B71 通信引脚定义
 ***************************************************************************************/
// CS引脚定义（片选）
#define      macGC9B71_CS_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define      macGC9B71_CS_CLK                RCC_APB2Periph_GPIOD    
#define      macGC9B71_CS_PORT               GPIOD   
#define      macGC9B71_CS_PIN                GPIO_Pin_7

// RS引脚定义（命令/数据选择）
#define      macGC9B71_DC_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define      macGC9B71_DC_CLK                RCC_APB2Periph_GPIOD    
#define      macGC9B71_DC_PORT               GPIOD   
#define      macGC9B71_DC_PIN                GPIO_Pin_11

// RST引脚定义（复位）
#define      macGC9B71_RST_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define      macGC9B71_RST_CLK               RCC_APB2Periph_GPIOE   
#define      macGC9B71_RST_PORT              GPIOE
#define      macGC9B71_RST_PIN               GPIO_Pin_1

// BK引脚定义（背光）
#define      macGC9B71_BK_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define      macGC9B71_BK_CLK                RCC_APB2Periph_GPIOD    
#define      macGC9B71_BK_PORT               GPIOD
#define      macGC9B71_BK_PIN                GPIO_Pin_12

/***************************************************************************************
 * 调试延时宏定义
 ***************************************************************************************/
#define      macDEBUG_DELAY()                

/***************************************************************************************
 * GC9B71 显示区域默认尺寸（圆形屏通常为240x240）
 * 注：根据实际屏幕规格调整，GC9B71常见分辨率为240x240或360x360
 ***************************************************************************************/
#define      macGC9B71_Default_Max_Width		  240     // 默认显示宽度
#define      macGC9B71_Default_Max_Heigth		240     // 默认显示高度

/***************************************************************************************
 * GC9B71 显示窗口起始位置和尺寸
 ***************************************************************************************/
#define      macGC9B71_DispWindow_X_Star		    0     // 显示窗口起始X坐标
#define      macGC9B71_DispWindow_Y_Star		    0     // 显示窗口起始Y坐标
#define      macGC9B71_DispWindow_COLUMN		    240   // 显示窗口列数（宽度）
#define      macGC9B71_DispWindow_PAGE		      240   // 显示窗口页数（高度）

/***************************************************************************************
 * 字符显示尺寸定义
 ***************************************************************************************/
#define      macWIDTH_EN_CHAR		               8	    // 英文字符宽度（像素）
#define      macHEIGHT_EN_CHAR		              16	  // 英文字符高度（像素）

#define      macWIDTH_CH_CHAR		                16	  // 中文字符宽度（像素）
#define      macHEIGHT_CH_CHAR		              16	  // 中文字符高度（像素）

// 获取中文字库的函数宏定义
#define      macGetGBKCode( ucBuffer, usChar )  GetGBKCode_from_sd( ucBuffer, usChar )

/***************************************************************************************
 * 常用颜色定义（RGB565格式）
 ***************************************************************************************/
#define      macBACKGROUND		                  macBLACK   // 默认背景色为黑色

#define      macWHITE		 		                  0xFFFF	   // 白色
#define      macBLACK                         0x0000	   // 黑色 
#define      macGREY                          0xF7DE	   // 灰色 
#define      macBLUE                          0x001F	   // 蓝色 
#define      macBLUE2                         0x051F	   // 浅蓝色 
#define      macRED                           0xF800	   // 红色 
#define      macMAGENTA                       0xF81F	   // 品红色/紫红色 
#define      macGREEN                         0x07E0	   // 绿色 
#define      macCYAN                          0x7FFF	   // 青色/蓝绿色 
#define      macYELLOW                        0xFFE0	   // 黄色 
#define      macBRED                          0xF81F    // 亮红色
#define      macGRED                          0xFFE0    // 绿色红色
#define      macGBLUE                         0x07FF    // 绿蓝色

/***************************************************************************************
 * GC9B71 命令定义
 ***************************************************************************************/
#define      macCMD_SetCoordinateX		 		    0x2A	     // 设置X坐标命令
#define      macCMD_SetCoordinateY		 		    0x2B	     // 设置Y坐标命令
#define      macCMD_SetPixel		 		          0x2C	     // 写内存命令（开始写入像素数据）

/***************************************************************************************
 * GC9B71 函数声明
 ***************************************************************************************/

/**
 * @brief  初始化GC9B71屏幕
 * @param  无
 * @retval 无
 */
void                     GC9B71_Init                    ( void );

/**
 * @brief  硬件复位GC9B71
 * @param  无
 * @retval 无
 */
void                     GC9B71_Rst                     ( void );

/**
 * @brief  控制背光开关
 * @param  enumState: ENABLE-开背光, DISABLE-关背光
 * @retval 无
 */
void                     GC9B71_BackLed_Control         ( FunctionalState enumState );

/**
 * @brief  发送命令（已通过comm_out实现）
 * @param  usCmd: 命令值
 * @retval 无
 */
__inline void            GC9B71_Write_Cmd               ( uint16_t usCmd );

/**
 * @brief  发送数据（已通过GC9B71_Write_Data实现）
 * @param  usData: 24位数据（RGB565格式）
 * @retval 无
 */
__inline void            GC9B71_Write_Data              ( uint32_t usData );

/**
 * @brief  读取数据（SPI模式可能需要实现）
 * @param  无
 * @retval 读取到的16位数据
 */
__inline uint16_t        GC9B71_Read_Data               ( void );

/**
 * @brief  设置GRAM扫描方向
 * @param  ucOption: 1-4种扫描方向选项
 * @retval 无
 */
void                     GC9B71_GramScan                ( uint8_t ucOtion );

/**
 * @brief  打开显示窗口
 * @param  usX: 起始X坐标
 * @param  usY: 起始Y坐标
 * @param  usWidth: 窗口宽度
 * @param  usHeight: 窗口高度
 * @retval 无
 */
void                     GC9B71_OpenWindow              ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight );

/**
 * @brief  清屏函数（填充指定区域）
 * @param  usX: 起始X坐标
 * @param  usY: 起始Y坐标
 * @param  usWidth: 区域宽度
 * @param  usHeight: 区域高度
 * @param  usColor: 填充颜色（24位RGB格式）
 * @retval 无
 */
void                     GC9B71_Clear                   ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint32_t usColor );

/**
 * @brief  绘制一个像素点
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  usColor: 像素颜色（RGB565格式）
 * @retval 无
 */
void                     GC9B71_SetPointPixel           ( uint16_t usX, uint16_t usY, uint16_t usColor );

/**
 * @brief  获取指定坐标的像素颜色
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @retval 像素颜色值（RGB565格式）
 */
uint16_t                 GC9B71_GetPointPixel           ( uint16_t usX , uint16_t usY );

/**
 * @brief  绘制线段（Bresenham算法）
 * @param  usX1: 起点X坐标
 * @param  usY1: 起点Y坐标
 * @param  usX2: 终点X坐标
 * @param  usY2: 终点Y坐标
 * @param  usColor: 线段颜色
 * @retval 无
 */
void                     GC9B71_DrawLine                ( uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2, uint16_t usColor );

/**
 * @brief  绘制矩形
 * @param  usX_Start: 起始X坐标
 * @param  usY_Start: 起始Y坐标
 * @param  usWidth: 宽度
 * @param  usHeight: 高度
 * @param  usColor: 颜色
 * @param  ucFilled: 是否填充（0-空心，1-实心）
 * @retval 无
 */
void                     GC9B71_DrawRectangle           ( uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint16_t usColor, uint8_t ucFilled );

/**
 * @brief  绘制圆形（Bresenham算法）
 * @param  usX_Center: 圆心X坐标
 * @param  usY_Center: 圆心Y坐标
 * @param  usRadius: 半径
 * @param  usColor: 颜色
 * @param  ucFilled: 是否填充（0-空心，1-实心）
 * @retval 无
 */
void                     GC9B71_DrawCircle              ( uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint16_t usColor, uint8_t ucFilled );

/**
 * @brief  显示英文字符
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  cChar: 要显示的字符
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void                     GC9B71_DispChar_EN             ( uint16_t usX, uint16_t usY, const char cChar, uint16_t usColor_Background, uint16_t usColor_Foreground );

/**
 * @brief  显示英文字符串
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  pStr: 字符串指针
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void                     GC9B71_DispString_EN           ( uint16_t usX, uint16_t usY, const char * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground );

/**
 * @brief  显示中文字符
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  usChar: 汉字内码
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void                     GC9B71_DispChar_CH             ( uint16_t usX, uint16_t usY, uint16_t usChar, uint16_t usColor_Background, uint16_t usColor_Foreground );

/**
 * @brief  显示中文字符串
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  pStr: 字符串指针
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void                     GC9B71_DispString_CH           ( uint16_t usX, uint16_t usY, const uint8_t * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground );

/**
 * @brief  显示中英文字符串（自动识别中英文）
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  pStr: 字符串指针
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void                     GC9B71_DispString_EN_CH        ( uint16_t usX, uint16_t usY, const uint8_t * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground );

/**
 * @brief  显示灰度渐变图案（测试用）
 * @param  无
 * @retval 无
 */
void disp_gray(void);

/**
 * @brief  显示图片
 * @param  无
 * @retval 无
 */
void pic_show(void);

/**
 * @brief  显示边框
 * @param  无
 * @retval 无
 */
void DispFrame(void);

#endif /* __BSP_GC9B71_LCD_H */
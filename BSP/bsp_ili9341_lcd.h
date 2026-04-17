#ifndef      __BSP_ILI9341_LCD_H
#define      __BSP_ILI9341_LCD_H

#include "stm32f10x.h"

/***************************************************************************************
 * FSMC地址映射说明：
 * 2^26 = 0X0400 0000 = 64MB，每个BANK有4个64MB的块 = 256MB
 * 64MB: FSMC_Bank1_NORSRAM1: 0X6000 0000 ~ 0X63FF FFFF
 * 64MB: FSMC_Bank1_NORSRAM2: 0X6400 0000 ~ 0X67FF FFFF
 * 64MB: FSMC_Bank1_NORSRAM3: 0X6800 0000 ~ 0X6BFF FFFF
 * 64MB: FSMC_Bank1_NORSRAM4: 0X6C00 0000 ~ 0X6FFF FFFF
 *
 * 选择BANK1-NORSRAM1作为TFT的地址空间，地址范围为0X6000 0000 ~ 0X63FF FFFF
 * FSMC_A16连接到LCD的DC（寄存器/数据选择）引脚
 * 寄存器访问地址 = 0X6000 0000
 * RAM访问地址 = 0X6002 0000 = 0X6000 0000 + 2^16 * 2 = 0X6000 0000 + 0X20000 = 0X6002 0000
 * 注意：选择不同的地址线时，地址需要重新计算
 ****************************************************************************************/

/******************************* ILI9341 显示接口 FSMC 宏定义 ***************************/
#define      macFSMC_Addr_ILI9341_CMD         ( ( uint32_t ) 0x60000000 )	    // FSMC_Bank1_NORSRAM1，LCD命令寄存器的地址
#define      macFSMC_Addr_ILI9341_DATA        ( ( uint32_t ) 0x60020000 )      // FSMC_Bank1_NORSRAM1，LCD数据寄存器的地址      

#define      macFSMC_Bank1_NORSRAMx           FSMC_Bank1_NORSRAM1              // 使用的FSMC BANK

/******************************* ILI9341 显示接口8080通信引脚定义 ***************************/
// 片选(CS)引脚配置
#define      macILI9341_CS_APBxClock_FUN      RCC_APB2PeriphClockCmd           // CS引脚时钟使能函数
#define      macILI9341_CS_CLK                RCC_APB2Periph_GPIOD             // CS引脚时钟
#define      macILI9341_CS_PORT               GPIOD                             // CS引脚端口
#define      macILI9341_CS_PIN                GPIO_Pin_7                        // CS引脚号

// 命令/数据选择(DC)引脚配置
#define      macILI9341_DC_APBxClock_FUN      RCC_APB2PeriphClockCmd           // DC引脚时钟使能函数
#define      macILI9341_DC_CLK                RCC_APB2Periph_GPIOD             // DC引脚时钟
#define      macILI9341_DC_PORT               GPIOD                             // DC引脚端口
#define      macILI9341_DC_PIN                GPIO_Pin_11                       // DC引脚号

// 复位(RST)引脚配置
#define      macILI9341_RST_APBxClock_FUN     RCC_APB2PeriphClockCmd           // RST引脚时钟使能函数
#define      macILI9341_RST_CLK               RCC_APB2Periph_GPIOE             // RST引脚时钟
#define      macILI9341_RST_PORT              GPIOE                             // RST引脚端口
#define      macILI9341_RST_PIN               GPIO_Pin_1                        // RST引脚号

// 背光(BK)引脚配置
#define      macILI9341_BK_APBxClock_FUN      RCC_APB2PeriphClockCmd           // BK引脚时钟使能函数
#define      macILI9341_BK_CLK                RCC_APB2Periph_GPIOD             // BK引脚时钟
#define      macILI9341_BK_PORT               GPIOD                             // BK引脚端口
#define      macILI9341_BK_PIN                GPIO_Pin_12                       // BK引脚号

/*************************************** 调试宏定义 ******************************************/
#define      macDEBUG_DELAY()                 // 调试延时宏（空实现）

/***************** ILI9341 显示屏幕默认参数（扫描方向为1时的屏幕宽度和高度） *********************/
#define      macILI9341_Default_Max_Width		  240     // 默认屏幕宽度（X轴方向）
#define      macILI9341_Default_Max_Heigth		320     // 默认屏幕高度（Y轴方向）

/***************************** ILI9341 显示窗口默认起始坐标和大小 ***************************/
#define      macILI9341_DispWindow_X_Star		    0     // 显示窗口起始X坐标
#define      macILI9341_DispWindow_Y_Star		    0     // 显示窗口起始Y坐标

#define      macILI9341_DispWindow_COLUMN		  240     // 显示窗口列数（宽度）
#define      macILI9341_DispWindow_PAGE		    320     // 显示窗口页数（高度）

/***************************** 在 ILI9341 显示区域上显示字符的大小 ***************************/
#define      macWIDTH_EN_CHAR		                 8	      // 英文字符宽度（像素）
#define      macHEIGHT_EN_CHAR		              16	    // 英文字符高度（像素）

#define      macWIDTH_CH_CHAR		                16	    // 中文字符宽度（像素）
#define      macHEIGHT_CH_CHAR		              16	    // 中文字符高度（像素）

// 获取中文字符字模数据的宏定义
// ucBuffer: 存储字模数据的缓冲区，usChar: 中文字符的内码
#define      macGetGBKCode( ucBuffer, usChar )  GetGBKCode_from_sd( ucBuffer, usChar )	  // 从SD卡读取中文字符的字模数据

/******************************* 定义 ILI9341 显示区域的常用颜色 ********************************/
#define      macBACKGROUND		                macBLACK   // 默认背景色（黑色）

// 常用颜色RGB565格式定义
#define      macWHITE		 		                  0xFFFF	   // 白色
#define      macBLACK                         0x0000	   // 黑色
#define      macGREY                          0xF7DE	   // 灰色
#define      macBLUE                          0x001F	   // 蓝色
#define      macBLUE2                         0x051F	   // 浅蓝色
#define      macRED                           0xF800	   // 红色
#define      macMAGENTA                       0xF81F	   // 品红色/紫红色
#define      macGREEN                         0x07E0	   // 绿色
#define      macCYAN                          0x7FFF	   // 青色
#define      macYELLOW                        0xFFE0	   // 黄色
#define      macBRED                          0xF81F     // 亮红色
#define      macGRED                          0xFFE0     // 亮红色（实际为黄色）
#define      macGBLUE                         0x07FF     // 亮蓝色

/******************************* 定义 ILI9341 相关命令 ********************************/
#define      macCMD_SetCoordinateX		 		    0x2A	     // 设置X坐标命令
#define      macCMD_SetCoordinateY		 		    0x2B	     // 设置Y坐标命令
#define      macCMD_SetPixel		 		          0x2C	     // 写像素命令

/********************************** 导出 ILI9341 函数 ***************************************/
void                     ILI9341_Init                    ( void );                                                    // ILI9341初始化函数
void                     ILI9341_Rst                     ( void );                                                    // ILI9341硬件复位函数
void                     ILI9341_BackLed_Control         ( FunctionalState enumState );                               // ILI9341背光控制函数
__inline void            ILI9341_Write_Cmd               ( uint16_t usCmd );                                          // 向ILI9341写命令（内联函数）
void                     ILI9341_Write_Data              ( uint16_t usData );                                         // 向ILI9341写数据
uint16_t                 ILI9341_Read_Data               ( void );                                                    // 从ILI9341读取数据
void                     ILI9341_GramScan                ( uint8_t ucOtion );                                         // 设置GRAM扫描方向
void                     ILI9341_OpenWindow              ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight );  // 打开显示窗口
void                     ILI9341_Clear                   ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint16_t usColor );  // 区域清屏填充
void                     ILI9341_SetPointPixel           ( uint16_t usX, uint16_t usY, uint16_t usColor );            // 绘制单个像素点
uint16_t                 ILI9341_GetPointPixel           ( uint16_t usX , uint16_t usY );                             // 获取指定坐标的像素颜色值
void                     ILI9341_DrawLine                ( uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2, uint16_t usColor );  // 绘制直线（Bresenham算法）
void                     ILI9341_DrawRectangle           ( uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint16_t usColor, uint8_t ucFilled );  // 绘制矩形（可选填充）
void                     ILI9341_DrawCircle              ( uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint16_t usColor, uint8_t ucFilled );  // 绘制圆形（Bresenham算法，可选填充）
void                     ILI9341_DispChar_EN             ( uint16_t usX, uint16_t usY, const char cChar, uint16_t usColor_Background, uint16_t usColor_Foreground );  // 显示单个英文字符
void                     ILI9341_DispString_EN           ( uint16_t usX, uint16_t usY, const char * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground );  // 显示英文字符串
void                     ILI9341_DispChar_CH             ( uint16_t usX, uint16_t usY, uint16_t usChar, uint16_t usColor_Background, uint16_t usColor_Foreground );  // 显示单个中文字符
void                     ILI9341_DispString_CH           ( uint16_t usX, uint16_t usY, const uint8_t * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground );  // 显示中文字符串
void                     ILI9341_DispString_EN_CH        ( uint16_t usX, uint16_t usY, const uint8_t * pStr, uint16_t usColor_Background, uint16_t usColor_Foreground );  // 显示中英文混合字符串

void                     disp_gray                       ( void );                                                    // 显示灰度渐变测试图案
void                     ILI9341_LVGL_Flush              ( uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint16_t *pColors );  // LVGL图形库专用刷新函数
void                     pic_show                        ( void );                                                    // 显示图片（需要实现）
void                     DISP9W                          ( void );                                                    // 显示9宫格棋盘格测试图案


#endif /* __BSP_ILI9341_ILI9341_H */

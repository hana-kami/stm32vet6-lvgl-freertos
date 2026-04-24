#ifndef __BSP_ST75161_LCD_H
#define __BSP_ST75161_LCD_H

#include "stm32f10x.h"
#include "bsp_sdfs_app.h"    // 获取SD卡字库数据

/* ==================== ST75161 驱动说明 ==================== */
/**
 * @file bsp_st75161_lcd.h
 * @brief ST75161 LCD屏幕驱动头文件（并行8080接口）
 * @note 适用于ST75161驱动IC的单色/灰度LCD屏幕
 */

/* ==================== FSMC 地址定义 ==================== */
/**
 * FSMC地址映射说明：
 * 2^26 = 0X0400 0000 = 64MB，每个BANK有4*64MB = 256MB
 * 64MB: FSMC_Bank1_NORSRAM1: 0X6000 0000 ~ 0X63FF FFFF
 * 64MB: FSMC_Bank1_NORSRAM2: 0X6400 0000 ~ 0X67FF FFFF
 * 64MB: FSMC_Bank1_NORSRAM3: 0X6800 0000 ~ 0X6BFF FFFF
 * 64MB: FSMC_Bank1_NORSRAM4: 0X6C00 0000 ~ 0X6FFF FFFF
 * 
 * 选择BANK1-NORSRAM1作为TFT地址范围：0X6000 0000 ~ 0X63FF FFFF
 * FSMC_A16连接LCD的DC(寄存器/数据选择)引脚
 * 寄存器访问地址 = 0X60000000
 * RAM访问地址 = 0X60020000 = 0X60000000 + 2^16*2
 */
#define macFSMC_Addr_ST75161_CMD         ((uint32_t)0x60000000)   // LCD命令寄存器地址
#define macFSMC_Addr_ST75161_DATA        ((uint32_t)0x60020000)   // LCD数据寄存器地址
#define macFSMC_Bank1_NORSRAMx           FSMC_Bank1_NORSRAM1

/* ==================== ST75161 通信引脚定义 ==================== */
// CS引脚定义（片选）
#define macST75161_CS_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define macST75161_CS_CLK                RCC_APB2Periph_GPIOD    
#define macST75161_CS_PORT               GPIOD   
#define macST75161_CS_PIN                GPIO_Pin_7

// DC引脚定义（命令/数据选择）
#define macST75161_DC_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define macST75161_DC_CLK                RCC_APB2Periph_GPIOD    
#define macST75161_DC_PORT               GPIOD   
#define macST75161_DC_PIN                GPIO_Pin_11

// RST引脚定义（复位）
#define macST75161_RST_APBxClock_FUN     RCC_APB2PeriphClockCmd
#define macST75161_RST_CLK               RCC_APB2Periph_GPIOE   
#define macST75161_RST_PORT              GPIOE
#define macST75161_RST_PIN               GPIO_Pin_1

// BK引脚定义（背光）
#define macST75161_BK_APBxClock_FUN      RCC_APB2PeriphClockCmd
#define macST75161_BK_CLK                RCC_APB2Periph_GPIOD    
#define macST75161_BK_PORT               GPIOD
#define macST75161_BK_PIN                GPIO_Pin_12

/* ==================== ST75161 控制引脚操作宏 ==================== */
// CS引脚
#define ST75161_CS_CLK                   RCC_APB2Periph_GPIOD    
#define ST75161_CS_PORT                  GPIOD   
#define ST75161_CS_PIN                   GPIO_Pin_7
#define ST75161_CS_CLR                   GPIOD->BRR = (1 << 7)
#define ST75161_CS_SET                   GPIOD->BSRR = (1 << 7)

// DC引脚（命令/数据选择）
#define ST75161_DC_CLK                   RCC_APB2Periph_GPIOD    
#define ST75161_DC_PORT                  GPIOD   
#define ST75161_DC_PIN                   GPIO_Pin_11
#define ST75161_DC_CLR                   GPIOD->BRR = (1 << 11)
#define ST75161_DC_SET                   GPIOD->BSRR = (1 << 11)

// WR引脚（写使能）
#define ST75161_WR_CLK                   RCC_APB2Periph_GPIOD    
#define ST75161_WR_PORT                  GPIOD   
#define ST75161_WR_PIN                   GPIO_Pin_5
#define ST75161_WR_CLR                   GPIOD->BRR = (1 << 5)
#define ST75161_WR_SET                   GPIOD->BSRR = (1 << 5)

// RD引脚（读使能）
#define ST75161_RD_CLK                   RCC_APB2Periph_GPIOD    
#define ST75161_RD_PORT                  GPIOD   
#define ST75161_RD_PIN                   GPIO_Pin_4
#define ST75161_RD_CLR                   GPIOD->BRR = (1 << 4)
#define ST75161_RD_SET                   GPIOD->BSRR = (1 << 4)

/* ==================== ST75161 数据引脚定义 ==================== */
// D0数据引脚
#define ST75161_D0_CLK                   RCC_APB2Periph_GPIOD    
#define ST75161_D0_PORT                  GPIOD   
#define ST75161_D0_PIN                   GPIO_Pin_14

// D1数据引脚
#define ST75161_D1_CLK                   RCC_APB2Periph_GPIOD    
#define ST75161_D1_PORT                  GPIOD   
#define ST75161_D1_PIN                   GPIO_Pin_15

// D2数据引脚
#define ST75161_D2_CLK                   RCC_APB2Periph_GPIOD    
#define ST75161_D2_PORT                  GPIOD   
#define ST75161_D2_PIN                   GPIO_Pin_0

// D3数据引脚
#define ST75161_D3_CLK                   RCC_APB2Periph_GPIOD    
#define ST75161_D3_PORT                  GPIOD   
#define ST75161_D3_PIN                   GPIO_Pin_1

// D4数据引脚
#define ST75161_D4_CLK                   RCC_APB2Periph_GPIOE    
#define ST75161_D4_PORT                  GPIOE   
#define ST75161_D4_PIN                   GPIO_Pin_7

// D5数据引脚
#define ST75161_D5_CLK                   RCC_APB2Periph_GPIOE    
#define ST75161_D5_PORT                  GPIOE   
#define ST75161_D5_PIN                   GPIO_Pin_8

// D6数据引脚
#define ST75161_D6_CLK                   RCC_APB2Periph_GPIOE    
#define ST75161_D6_PORT                  GPIOE   
#define ST75161_D6_PIN                   GPIO_Pin_9

// D7数据引脚
#define ST75161_D7_CLK                   RCC_APB2Periph_GPIOE    
#define ST75161_D7_PORT                  GPIOE   
#define ST75161_D7_PIN                   GPIO_Pin_10

/* ==================== 屏幕尺寸定义 ==================== */
#define macST75161_Default_Max_Width     160     // 默认显示宽度
#define macST75161_Default_Max_Heigth    160     // 默认显示高度

/* ==================== 显示窗口起始位置和尺寸 ==================== */
#define macST75161_DispWindow_X_Star     0       // 显示窗口起始X坐标
#define macST75161_DispWindow_Y_Star     0       // 显示窗口起始Y坐标
#define macST75161_DispWindow_COLUMN     160     // 显示窗口列数（宽度）
#define macST75161_DispWindow_PAGE       160     // 显示窗口页数（高度）

/* ==================== 字符显示尺寸定义 ==================== */
#define macWIDTH_EN_CHAR                 8       // 英文字符宽度（像素）
#define macHEIGHT_EN_CHAR                16      // 英文字符高度（像素）

#define macWIDTH_CH_CHAR                 16      // 中文字符宽度（像素）
#define macHEIGHT_CH_CHAR                16      // 中文字符高度（像素）

// 获取中文字库的函数宏定义
#define macGetGBKCode(ucBuffer, usChar)  GetGBKCode_from_sd(ucBuffer, usChar)

/* ==================== 颜色定义（RGB565格式） ==================== */
#define macBACKGROUND                    macBLACK   // 默认背景色为黑色

#define macWHITE                         0xFFFF    // 白色
#define macBLACK                         0x0000    // 黑色
#define macGREY                          0xF7DE    // 灰色
#define macBLUE                          0x001F    // 蓝色
#define macBLUE2                         0x051F    // 浅蓝色
#define macRED                           0xF800    // 红色
#define macMAGENTA                       0xF81F    // 品红色
#define macGREEN                         0x07E0    // 绿色
#define macCYAN                          0x7FFF    // 青色
#define macYELLOW                        0xFFE0    // 黄色
#define macBRED                          0xF81F    // 亮红色
#define macGRED                          0xFFE0    // 绿红色
#define macGBLUE                         0x07FF    // 绿蓝色

/* ==================== ST75161 命令定义 ==================== */
#define macCMD_SetCoordinateX            0x2A      // 设置X坐标命令
#define macCMD_SetCoordinateY            0x2B      // 设置Y坐标命令
#define macCMD_SetPixel                  0x2C      // 写内存命令（开始写入像素数据）


/* ==================== ST75161 函数声明 ==================== */

/**
 * @brief  初始化ST75161屏幕
 * @param  无
 * @retval 无
 */
void ST75161_Init(void);

/**
 * @brief  硬件复位ST75161
 * @param  无
 * @retval 无
 */
void ST75161_Rst(void);

/**
 * @brief  控制背光开关
 * @param  enumState: ENABLE-开背光, DISABLE-关背光
 * @retval 无
 */
void ST75161_BackLed_Control(FunctionalState enumState);

/**
 * @brief  发送命令（通过comm_out实现）
 * @param  usCmd: 命令值
 * @retval 无
 */
__inline void ST75161_Write_Cmd(uint16_t usCmd);

/**
 * @brief  发送数据
 * @param  usData: 16位数据
 * @retval 无
 */
__inline void ST75161_Write_Data(uint16_t usData);

/**
 * @brief  读取数据
 * @param  无
 * @retval 读取到的16位数据
 */
__inline uint16_t ST75161_Read_Data(void);

/**
 * @brief  设置GRAM扫描方向
 * @param  ucOption: 1-4种扫描方向选项
 * @retval 无
 */
void ST75161_GramScan(uint8_t ucOption);

/**
 * @brief  打开显示窗口
 * @param  usX: 起始X坐标
 * @param  usY: 起始Y坐标
 * @param  usWidth: 窗口宽度
 * @param  usHeight: 窗口高度
 * @retval 无
 */
void ST75161_OpenWindow(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight);

/**
 * @brief  区域清屏填充
 * @param  usX: 起始X坐标
 * @param  usY: 起始Y坐标
 * @param  usWidth: 区域宽度
 * @param  usHeight: 区域高度
 * @param  usColor: 填充颜色
 * @retval 无
 */
void ST75161_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight, uint16_t usColor);

/**
 * @brief  绘制一个像素点
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  usColor: 像素颜色
 * @retval 无
 */
void ST75161_SetPointPixel(uint16_t usX, uint16_t usY, uint16_t usColor);

/**
 * @brief  获取指定坐标的像素颜色
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @retval 像素颜色值
 */
uint16_t ST75161_GetPointPixel(uint16_t usX, uint16_t usY);

/**
 * @brief  绘制线段（Bresenham算法）
 * @param  usX1: 起点X坐标
 * @param  usY1: 起点Y坐标
 * @param  usX2: 终点X坐标
 * @param  usY2: 终点Y坐标
 * @param  usColor: 线段颜色
 * @retval 无
 */
void ST75161_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2, uint16_t usColor);

/**
 * @brief  绘制矩形
 * @param  usX_Start: 起始X坐标
 * @param  usY_Start: 起始Y坐标
 * @param  usWidth: 矩形宽度
 * @param  usHeight: 矩形高度
 * @param  usColor: 矩形颜色
 * @param  ucFilled: 是否填充（0-空心，1-实心）
 * @retval 无
 */
void ST75161_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, 
                           uint16_t usHeight, uint16_t usColor, uint8_t ucFilled);

/**
 * @brief  绘制圆形（Bresenham算法）
 * @param  usX_Center: 圆心X坐标
 * @param  usY_Center: 圆心Y坐标
 * @param  usRadius: 圆的半径
 * @param  usColor: 圆的颜色
 * @param  ucFilled: 是否填充（0-空心，1-实心）
 * @retval 无
 */
void ST75161_DrawCircle(uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, 
                        uint16_t usColor, uint8_t ucFilled);

/**
 * @brief  显示英文字符
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  cChar: 要显示的字符
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void ST75161_DispChar_EN(uint16_t usX, uint16_t usY, const char cChar, 
                         uint16_t usColor_Background, uint16_t usColor_Foreground);

/**
 * @brief  显示英文字符串
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  pStr: 字符串指针
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void ST75161_DispString_EN(uint16_t usX, uint16_t usY, const char *pStr, 
                           uint16_t usColor_Background, uint16_t usColor_Foreground);

/**
 * @brief  显示中文字符
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  usChar: 汉字内码
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void ST75161_DispChar_CH(uint16_t usX, uint16_t usY, uint16_t usChar, 
                         uint16_t usColor_Background, uint16_t usColor_Foreground);

/**
 * @brief  显示中文字符串
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  pStr: 字符串指针
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void ST75161_DispString_CH(uint16_t usX, uint16_t usY, const uint8_t *pStr, 
                           uint16_t usColor_Background, uint16_t usColor_Foreground);

/**
 * @brief  显示中英文字符串（自动识别中英文）
 * @param  usX: X坐标
 * @param  usY: Y坐标
 * @param  pStr: 字符串指针
 * @param  usColor_Background: 背景色
 * @param  usColor_Foreground: 前景色
 * @retval 无
 */
void ST75161_DispString_EN_CH(uint16_t usX, uint16_t usY, const uint8_t *pStr, 
                              uint16_t usColor_Background, uint16_t usColor_Foreground);

/* ==================== 底层通信函数 ==================== */
/**
 * @brief  发送命令（底层）
 * @param  usCmd: 命令值
 * @retval 无
 */
__inline void comm_out(uint16_t usCmd);

/**
 * @brief  发送数据（底层8位）
 * @param  x: 8位数据
 * @retval 无
 */
__inline void data_out(uint16_t x);

/**
 * @brief  发送数据（底层）
 * @param  usCmd: 16位数据
 * @retval 无
 */
__inline void DATA_WR(uint16_t usCmd);

/* ==================== 私有功能函数 ==================== */
/**
 * @brief  清空ICON DDRAM
 * @param  无
 * @retval 无
 */
void clear_ICON_DDRAM(void);

/**
 * @brief  清空显示DDRAM
 * @param  无
 * @retval 无
 */
void clealddram(void);

/**
 * @brief  显示图形数据
 * @param  gph: 图形数据指针
 * @retval 无
 */
void disp_graphics1(const unsigned char *gph);

/**
 * @brief  显示灰度渐变图案
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
 * @brief  显示9宫格棋盘格图案
 * @param  无
 * @retval 无
 */
void DISP9W(void);

/**
 * @brief  显示像素行数据
 * @param  dat1: 数据1
 * @param  dat2: 数据2
 * @retval 无
 */
void disp_panxl(uint16_t dat1, uint16_t dat2);

#endif /* __BSP_ST75161_LCD_H */
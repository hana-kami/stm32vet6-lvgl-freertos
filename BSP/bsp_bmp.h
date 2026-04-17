#ifndef __BSP_BMP_H
#define __BSP_BMP_H

#include "stm32f10x.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * BMP图片处理头文件
 * 功能：提供BMP格式图片的显示和屏幕截图功能
 * 依赖：STM32F10x标准库、FatFs文件系统、LCD驱动
 *============================================================================*/

/**
 * @brief 计算图像行字节数的宏定义（4字节对齐）
 * @param bits 每行像素的总位数（宽度 × 每像素位数）
 * @retval 对齐后的行字节数（4的倍数）
 * @note BMP格式要求每行数据的字节数必须是4的倍数
 *       例如：宽度240像素×24位色=720位=90字节，对齐后为92字节
 */
#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)

/*============================================================================
 * 基础数据类型重定义（与Windows BMP文件格式保持一致）
 *============================================================================*/
typedef unsigned char BYTE;      // 无符号8位整数
typedef unsigned short WORD;     // 无符号16位整数
typedef unsigned long DWORD;     // 无符号32位整数
typedef long LONG;               // 有符号32位整数

/*============================================================================
 * BMP文件结构体定义（必须与Windows BMP文件格式完全一致）
 *============================================================================*/

/**
 * @brief BMP位图文件头结构体（共14字节）
 * @note 此结构体对应BMP文件开头的14字节
 */
typedef struct tagBITMAPFILEHEADER 
{   
    DWORD bfSize;           // 文件总大小（字节），偏移地址0x02，4字节
    WORD  bfReserved1;      // 保留字，必须为0，偏移地址0x06，2字节
    WORD  bfReserved2;      // 保留字，必须为0，偏移地址0x08，2字节
    DWORD bfOffBits;        // 位图数据相对于文件头的偏移量（字节），偏移地址0x0A，4字节
} BITMAPFILEHEADER, tagBITMAPFILEHEADER;

/**
 * @brief BMP位图信息头结构体（共40字节）
 * @note 此结构体紧跟在文件头之后，描述图像的具体信息
 */
typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;           // 本结构体的大小（40字节），偏移地址0x0E，4字节
    LONG  biWidth;          // 图像宽度（像素），偏移地址0x12，4字节
    LONG  biHeight;         // 图像高度（像素），偏移地址0x16，4字节
    WORD  biPlanes;         // 目标设备平面数，必须为1，偏移地址0x1A，2字节
    WORD  biBitCount;       // 每像素位数（1/4/8/16/24/32），偏移地址0x1C，2字节
    DWORD biCompression;    // 压缩类型（0=不压缩，1=RLE8，2=RLE4），偏移地址0x1E，4字节
    DWORD biSizeImage;      // 位图数据占用的字节数（可设为0），偏移地址0x22，4字节
    LONG  biXPelsPerMeter;  // 水平分辨率（像素/米），偏移地址0x26，4字节
    LONG  biYPelsPerMeter;  // 垂直分辨率（像素/米），偏移地址0x2A，4字节
    DWORD biClrUsed;        // 实际使用的颜色数（0表示使用所有），偏移地址0x2E，4字节
    DWORD biClrImportant;   // 重要颜色数（0表示都重要），偏移地址0x32，4字节
} BITMAPINFOHEADER, tagBITMAPINFOHEADER;

/**
 * @brief RGB颜色表结构体（用于索引色图像）
 * @note 仅当biBitCount <= 8时需要使用颜色表
 */
typedef struct tagRGBQUAD 
{
    BYTE rgbBlue;           // 蓝色分量
    BYTE rgbGreen;          // 绿色分量
    BYTE rgbRed;            // 红色分量
    BYTE rgbReserved;       // 保留字节（通常为0）
} RGBQUAD, tagRGBQUAD;

/**
 * @brief RGB像素数据结构体（24位真彩色）
 * @note 每个像素占3字节，存储顺序为B、G、R
 */
typedef struct RGB_PIXEL
{
    unsigned char rgbBlue;   // 蓝色分量（0-255）
    unsigned char rgbGreen;  // 绿色分量（0-255）
    unsigned char rgbRed;    // 红色分量（0-255）
} RGB_PIXEL;

/*============================================================================
 * RGB565格式转换宏定义
 *============================================================================*/

/**
 * @brief 从RGB565格式中提取红色分量并转换为8位
 * @param RGB565 16位RGB565格式的颜色值（位15-11:R, 10-5:G, 4-0:B）
 * @retval 8位红色分量（0-255）
 * @note RGB565红色占5位，左移3位扩展为8位（乘以8）
 */
#define GETR_FROM_RGB16(RGB565)  ((unsigned char)((((unsigned short int)RGB565) >> 11) << 3))

/**
 * @brief 从RGB565格式中提取绿色分量并转换为8位
 * @param RGB565 16位RGB565格式的颜色值
 * @retval 8位绿色分量（0-255）
 * @note RGB565绿色占6位，左移2位扩展为8位（乘以4）
 */
#define GETG_FROM_RGB16(RGB565)  ((unsigned char)((((unsigned short int)(RGB565 & 0x07FF)) >> 5) << 2))

/**
 * @brief 从RGB565格式中提取蓝色分量并转换为8位
 * @param RGB565 16位RGB565格式的颜色值
 * @retval 8位蓝色分量（0-255）
 * @note RGB565蓝色占5位，左移3位扩展为8位（乘以8）
 */
#define GETB_FROM_RGB16(RGB565)  ((unsigned char)(((unsigned short int)(RGB565 & 0x001F)) << 3))

/**
 * @brief RGB24格式转RGB16格式宏定义
 * @param R 红色分量（0-255）
 * @param G 绿色分量（0-255）
 * @param B 蓝色分量（0-255）
 * @retval RGB565格式的颜色值（16位）
 * @note 转换规则：R高5位，G中间6位，B低5位
 */
#define RGB24TORGB16(R,G,B) ((unsigned short int)((((R)>>3)<<11) | (((G)>>2)<<5) | ((B)>>3)))

/*============================================================================
 * 编译器相关设置
 *============================================================================*/
#pragma diag_suppress 870  // 抑制"invalid multibyte character sequence"警告
                           // 用于支持中文字符串编译

/*============================================================================
 * 外部函数声明
 *============================================================================*/

/******************************* BMP图片显示相关 ********************************/
typedef struct {
    uint16_t x;          // 显示X坐标
    uint16_t y;          // 显示Y坐标
    uint8_t *filename;   // 文件名
} BMP_Info_t;

// 序列帧播放控制结构体
typedef struct {
    uint16_t start_index;    // 起始帧序号
    uint16_t end_index;      // 结束帧序号
    uint16_t current_frame;  // 当前帧
    uint16_t delay_ms;       // 帧延迟(ms)
    uint8_t loop;            // 是否循环
    uint8_t playing;         // 是否播放中
    char path_prefix[50];    // 文件路径前缀，如 "/"
    char file_format[10];    // 文件名格式，如 ".bmp"
} FrameSequence_t;


// 全局序列帧播放器实例
static FrameSequence_t g_frame_seq;

// 函数声明
void Lcd_Show_bmp(uint16_t usX, uint16_t usY, const char *pPath);
void FrameSeq_Init(FrameSequence_t *seq, uint16_t start, uint16_t end, uint16_t delay, uint8_t loop);
void FrameSeq_SetPath(FrameSequence_t *seq, const char *prefix, const char *format);
void FrameSeq_Play(FrameSequence_t *seq, uint16_t x, uint16_t y);
void FrameSeq_Stop(FrameSequence_t *seq);
uint8_t FrameSeq_Update(FrameSequence_t *seq, uint16_t x, uint16_t y);

/**
 * @brief  屏幕截图，将LCD指定区域保存为BMP文件
 * @param  x 截图区域起始X坐标
 * @param  y 截图区域起始Y坐标
 * @param  Width 截图区域宽度（像素）
 * @param  Height 截图区域高度（像素）
 * @param  filename 保存的文件名（不含扩展名，自动添加.bmp）
 * @retval 返回值说明：
 *         @retval 0      截图成功
 *         @retval 8      文件已存在（FR_EXIST）
 *         @retval -1     截图失败（文件打开错误或其他错误）
 * @note 保存的文件格式：24位真彩色BMP
 * @note 文件保存位置：SD卡根目录
 * @note 文件名示例：filename="test" → 保存为 "0:/test.bmp"
 */
int Screen_shot(uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, uint8_t *filename);

#endif /* __BSP_BMP_H */

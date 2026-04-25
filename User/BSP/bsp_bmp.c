#include "bsp_bmp.h"
#include "ff.h"
#include "bsp_lcd.h"  // 改为包含统一接口头文件
//#include "FreeRTOS.h"     
//#include "task.h"         

/**
 * @brief RGB24格式转RGB16格式宏定义
 * @param R 红色分量（0-255）
 * @param G 绿色分量（0-255）
 * @param B 蓝色分量（0-255）
 * @retval RGB565格式的颜色值（16位）
 */
#define RGB24TORGB16(R,G,B) ((unsigned short int)((((R)>>3)<<11) | (((G)>>2)<<5) | ((B)>>3)))

/* 全局变量定义 */
BYTE pColorData[960];					/* 一行像素的颜色数据缓冲区，最大支持320像素 * 3字节 = 960 */
FATFS bmpfs[2];                         /* FATFS文件系统对象数组，用于SD卡文件系统 */
FIL bmpfsrc, bmpfdst;                   /* 文件对象：bmpfsrc用于读文件，bmpfdst用于写文件 */
FRESULT bmpres;                         /* FATFS操作结果码 */

/* 调试打印宏定义 */
#define BMP_DEBUG_PRINTF(FORMAT,...)  //printf(FORMAT,##__VA_ARGS__)	 

/**
 * @brief  打印BMP文件头信息（调试用）
 * @param  pBmpHead BMP文件头结构体指针
 * @retval 无
 */
static void showBmpHead(BITMAPFILEHEADER* pBmpHead)
{
    BMP_DEBUG_PRINTF("位图文件头:\r\n");
    BMP_DEBUG_PRINTF("文件大小:%d\r\n",(*pBmpHead).bfSize);
    BMP_DEBUG_PRINTF("保留字1:%d\r\n",(*pBmpHead).bfReserved1);
    BMP_DEBUG_PRINTF("保留字2:%d\r\n",(*pBmpHead).bfReserved2);
    BMP_DEBUG_PRINTF("实际位图数据的偏移字节数:%d\r\n",(*pBmpHead).bfOffBits);
    BMP_DEBUG_PRINTF("\r\n");	
}

/**
 * @brief  打印BMP位图信息头（调试用）
 * @param  pBmpInforHead BMP信息头结构体指针
 * @retval 无
 */
static void showBmpInforHead(tagBITMAPINFOHEADER* pBmpInforHead)
{
    BMP_DEBUG_PRINTF("位图信息头:\r\n");
    BMP_DEBUG_PRINTF("结构体的长度:%d\r\n",(*pBmpInforHead).biSize);
    BMP_DEBUG_PRINTF("位图宽度:%d\r\n",(*pBmpInforHead).biWidth);
    BMP_DEBUG_PRINTF("位图高度:%d\r\n",(*pBmpInforHead).biHeight);
    BMP_DEBUG_PRINTF("biPlanes平面数:%d\r\n",(*pBmpInforHead).biPlanes);
    BMP_DEBUG_PRINTF("biBitCount每个像素的位数:%d\r\n",(*pBmpInforHead).biBitCount);
    BMP_DEBUG_PRINTF("压缩方式:%d\r\n",(*pBmpInforHead).biCompression);
    BMP_DEBUG_PRINTF("biSizeImage实际位图数据占用的字节数:%d\r\n",(*pBmpInforHead).biSizeImage);
    BMP_DEBUG_PRINTF("X方向分辨率:%d\r\n",(*pBmpInforHead).biXPelsPerMeter);
    BMP_DEBUG_PRINTF("Y方向分辨率:%d\r\n",(*pBmpInforHead).biYPelsPerMeter);
    BMP_DEBUG_PRINTF("使用的颜色数:%d\r\n",(*pBmpInforHead).biClrUsed);
    BMP_DEBUG_PRINTF("重要颜色数:%d\r\n",(*pBmpInforHead).biClrImportant);
    BMP_DEBUG_PRINTF("\r\n");
}

/**
 * @brief  屏幕截图功能
 * @param  x 起始X坐标
 * @param  y 起始Y坐标
 * @param  Width 截图宽度
 * @param  Height 截图高度
 * @param  filename 保存的文件名
 * @retval 0:成功, FR_EXIST:文件已存在, -1:失败
 */
int Screen_shot( uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, uint8_t * filename)
{
	/* BMP文件头，共54字节 */
	unsigned char header[54] =
	{
		0x42, 0x4d,        // 0-1: BM标识
		0, 0, 0, 0,        // 2-5: 文件大小
		0, 0,              // 6-7: 保留字1
		0, 0,              // 8-9: 保留字2
		54, 0, 0, 0,       // 10-13: 数据偏移量（54字节）
		40, 0, 0, 0,       // 14-17: 信息头大小（40字节）
		0, 0, 0, 0,        // 18-21: 图像宽度
		0, 0, 0, 0,        // 22-25: 图像高度
		1, 0,              // 26-27: 平面数（必须为1）
		24, 0,             // 28-29: 像素位数（24位真彩色）
		0, 0, 0, 0,        // 30-33: 压缩方式（0表示不压缩）
		0, 0, 0, 0,        // 34-37: 图像大小（可设为0）
		0, 0, 0, 0,        // 38-41: X方向分辨率
		0, 0, 0, 0,        // 42-45: Y方向分辨率
		0, 0, 0, 0,        // 46-49: 使用的颜色数（0表示默认）
		0, 0, 0, 0         // 50-53: 重要颜色数（0表示都重要）
	};
	
	int i;                          // 行循环变量
	int j;                          // 列循环变量
	long file_size;                 // 文件总大小
	long width;                     // 图像宽度
	long height;                    // 图像高度
	unsigned char r, g, b;          // RGB颜色分量
	unsigned char tmp_name[30];     // 临时文件名缓冲区
	unsigned int mybw;              // 实际写入的字节数
	unsigned int read_data;         // 从LCD读取的像素数据
	char kk[4] = {0, 0, 0, 0};      // 用于对齐填充的4字节0数据
	uint8_t ucAlign;                // 每行需要填充的字节数（4字节对齐用）
	
	/* 计算文件总大小 = 像素数据 + 行填充字节 + 文件头 */
	file_size = (long)Width * (long)Height * 3 + Height * (Width % 4) + 54;		

	/* 设置文件大小到文件头（4字节，小端格式）*/
	header[2] = (unsigned char)(file_size & 0x000000ff);
	header[3] = (file_size >> 8) & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;
	
	/* 设置图像宽度到文件头（4字节，小端格式）*/
	width = Width;	
	header[18] = width & 0x000000ff;
	header[19] = (width >> 8) & 0x000000ff;
	header[20] = (width >> 16) & 0x000000ff;
	header[21] = (width >> 24) & 0x000000ff;
	
	/* 设置图像高度到文件头（4字节，小端格式）*/
	height = Height;
	header[22] = height & 0x000000ff;
	header[23] = (height >> 8) & 0x000000ff;
	header[24] = (height >> 16) & 0x000000ff;
	header[25] = (height >> 24) & 0x000000ff;
	
	/* 按指定格式组合完整文件名 */
	sprintf((char*)tmp_name, "0:%s.bmp", filename);
	
	/* 创建新文件 */
	bmpres = f_open( &bmpfsrc, (char*)tmp_name, FA_CREATE_NEW | FA_WRITE );
	f_close(&bmpfsrc);
	
	/* 以读写方式重新打开已存在的文件 */
	bmpres = f_open( &bmpfsrc, (char*)tmp_name, FA_OPEN_EXISTING | FA_WRITE );

	if ( bmpres == FR_OK )  // 文件打开成功
	{    
		/* 写入BMP文件头 */
		bmpres = f_write(&bmpfsrc, header, sizeof(unsigned char) * 54, &mybw);		
		
		/* 计算每行需要填充的字节数（使行字节数为4的倍数）*/
		ucAlign = Width % 4;
		
		/* 逐行读取LCD像素数据并写入文件 */
		for(i = 0; i < Height; i++)
		{
			for(j = 0; j < Width; j++)
			{					
				/* 获取LCD指定坐标的像素颜色值（注意BMP存储顺序为从底向上）*/
				read_data = LCD_GetPoint(x + j, y + Height - 1 - i);					
				
				/* 从RGB565格式中提取RGB分量 */
				r = GETR_FROM_RGB16(read_data);
				g = GETG_FROM_RGB16(read_data);
				b = GETB_FROM_RGB16(read_data);

				/* BMP格式存储顺序为BGR */
				f_write(&bmpfsrc, &b, sizeof(unsigned char), &mybw);
				f_write(&bmpfsrc, &g, sizeof(unsigned char), &mybw);
				f_write(&bmpfsrc, &r, sizeof(unsigned char), &mybw);
			}
			
			/* 行字节数不是4的倍数时需要填充0 */
			if( ucAlign )
				f_write(&bmpfsrc, kk, sizeof(unsigned char) * (ucAlign), &mybw);
		}

		f_close(&bmpfsrc);
		return 0;  // 截图成功
	}
	else if ( bmpres == FR_EXIST )
		return FR_EXIST;
	else
		return -1;
}

/**
 * @brief  显示BMP图片（统一接口版本）
 * @param  x 显示起始X坐标
 * @param  y 显示起始Y坐标
 * @param  pic_name BMP文件的完整路径
 * @retval 无
 */
void Lcd_show_bmp ( uint16_t x, uint16_t y, uint8_t * pic_name )
{
	int i, j, k;
	int width, height, l_width;

	BYTE red, green, blue;
	BITMAPFILEHEADER bitHead;
	BITMAPINFOHEADER bitInfoHead;
	WORD fileType;

	unsigned int read_num;
	unsigned char tmp_name[20];
	sprintf((char*)tmp_name,"0:%s",pic_name);

	f_mount(0, &bmpfs[0]);
	BMP_DEBUG_PRINTF("file mount ok \r\n");    
	bmpres = f_open( &bmpfsrc , (char *)tmp_name, FA_OPEN_EXISTING | FA_READ);
	
	if(bmpres == FR_OK)
	{
		BMP_DEBUG_PRINTF("Open file success\r\n");

		/* 读取文件类型标识（2字节）*/         
		f_read(&bmpfsrc, &fileType, sizeof(WORD), &read_num);     

		/* 判断是否为BMP文件 "BM" */
		if(fileType != 0x4d42)
		{
			BMP_DEBUG_PRINTF("file is not .bmp file!\r\n");
			return;
		}
		else
		{
			BMP_DEBUG_PRINTF("Ok this is .bmp file\r\n");	
		}        

		/* 读取BMP文件头信息 */
		f_read(&bmpfsrc, &bitHead, sizeof(tagBITMAPFILEHEADER), &read_num);        
		showBmpHead(&bitHead);

		/* 读取位图信息头信息 */
		f_read(&bmpfsrc, &bitInfoHead, sizeof(BITMAPINFOHEADER), &read_num);        
		showBmpInforHead(&bitInfoHead);
	}    
	else
	{
		BMP_DEBUG_PRINTF("file open fail!\r\n");
		/* 文件打开失败时，用不同颜色清屏提示错误类型 */
		switch(bmpres)
		{
			case FR_DISK_ERR:          // 1 
				LCD_Clear(0xF800); // 红色
				break;
			
			case FR_INT_ERR:           // 2 
				LCD_Clear(0xFFE0); // 黄色
				break;
			
			case FR_NOT_READY:         // 4
				LCD_Clear(0x07E0); // 绿色
				break;
			
			case FR_NO_FILE:           // 5 
				LCD_Clear(0x001F); // 蓝色
				break;
			
			case FR_NO_PATH:           // 6
				LCD_Clear(0x07FF); // 青色
				break;
			
			case FR_INVALID_NAME:      // 7
				LCD_Clear(0xF81F); // 品红
				break;
			
			case FR_DENIED:            // 9
				LCD_Clear(0xC618); // 灰色
				break;
			
			case FR_INVALID_OBJECT:    // 13
				LCD_Clear(0x0000); // 黑色
				break;
			
			default:                   // 其他错误
				LCD_Clear(0xFFFF); // 白色
				break;
		}
		return;
	}    

	width = bitInfoHead.biWidth;
	height = bitInfoHead.biHeight;

	/* 计算位图的实际宽度并确保它为4的倍数 */
	l_width = WIDTHBYTES(width * bitInfoHead.biBitCount);	

	if(l_width > 720)
	{
		BMP_DEBUG_PRINTF("\n SORRY, PIC IS TOO BIG (<=240)\n");
		return;
	}
	
	/* 设置LCD扫描方向为正常方向 */
	// LCD_SetScanDirection(1);
	
	/* 打开一个图片大小的窗口 */
	LCD_SetWindow(x, y, width, height);
    LCD_COMMOUT(macCMD_SetPixel);	
	/* 判断是否为24位真彩色图 */
	if( bitInfoHead.biBitCount >= 24 )
	{
		for ( i = 0; i < height; i ++ )
		{
			f_lseek(&bmpfsrc, bitHead.bfOffBits + (height - i - 1) * l_width);	
			
			/* 读取一行BMP数据到缓冲区 */
			f_read(&bmpfsrc, pColorData, l_width, &read_num);

			for(j = 0; j < width; j++)
			{
				k = j * 3;
				blue = pColorData[k+2];
				green = pColorData[k+1];
				red = pColorData[k];
				LCD_Write_Data(RGB24TORGB16(red, green, blue));
			}            
		}        
	}    
	else 
	{        
		BMP_DEBUG_PRINTF("SORRY, THIS PIC IS NOT A 24BITS REAL COLOR");
		return ;
	}
	
	f_close(&bmpfsrc);  
}
/**
 * @brief  显示BMP图片（通用版本，自动适配彩色屏和单色屏）
 * @param  usX: 显示起始X坐标
 * @param  usY: 显示起始Y坐标
 * @param  pPath: BMP文件的完整路径
 * @retval 0:成功, 非0:失败
 */
uint8_t Lcd_Show_bmp(uint16_t usX, uint16_t usY, const char *pPath)
{
    int i, j, k;
    int width, height, l_width;
    uint16_t color_16bit;
    
    BYTE red, green, blue;
    BYTE gray;
    BITMAPFILEHEADER bitHead;
    BITMAPINFOHEADER bitInfoHead;
    WORD fileType;
    
    unsigned int read_num;
    unsigned char tmp_name[20];
    BYTE palette[8];
    BYTE pixel;
    
    /* 单色屏相关变量 */
    int is_mono_screen = 0;
    uint16_t screen_width = LCD_GetWidth();
    uint16_t screen_height = LCD_GetHeight();
    uint16_t color0, color1;
    int byte_idx, bit_idx;
    int color0_is_white, color1_is_white;
    
    #if (CURRENT_LCD_TYPE == LCD_TYPE_ST75161)
        is_mono_screen = 1;
    #endif
    
    sprintf((char*)tmp_name, "0:%s", pPath);
    
    BMP_DEBUG_PRINTF("Open BMP file: %s\r\n", tmp_name);
    
    /* 打开BMP文件 */
    bmpres = f_open(&bmpfsrc, (char *)tmp_name, FA_OPEN_EXISTING | FA_READ);
    
    if(bmpres != FR_OK)
    {
        BMP_DEBUG_PRINTF("File open fail! Error: %d\r\n", bmpres);
        return 1;
    }
    
    /* 读取文件类型标识 */
    f_read(&bmpfsrc, &fileType, sizeof(WORD), &read_num);
    
    if(fileType != 0x4d42)
    {
        BMP_DEBUG_PRINTF("Not a BMP file!\r\n");
        f_close(&bmpfsrc);
        return 2;
    }
    
    /* 读取BMP文件头 */
    f_read(&bmpfsrc, &bitHead, sizeof(tagBITMAPFILEHEADER), &read_num);
    showBmpHead(&bitHead);
    
    /* 读取位图信息头 */
    f_read(&bmpfsrc, &bitInfoHead, sizeof(BITMAPINFOHEADER), &read_num);
    showBmpInforHead(&bitInfoHead);
    
    /* 获取图片尺寸 */
    width = bitInfoHead.biWidth;
    height = bitInfoHead.biHeight;
    
    BMP_DEBUG_PRINTF("Image size: %dx%d, BitCount: %d\r\n", width, height, bitInfoHead.biBitCount);
    
    /* 检查是否超出屏幕范围 */
    if(usX >= screen_width || usY >= screen_height)
    {
        BMP_DEBUG_PRINTF("Position out of screen!\r\n");
        f_close(&bmpfsrc);
        return 3;
    }
    
    /* 裁剪超出屏幕的部分 */
    if(usX + width > screen_width)
        width = screen_width - usX;
    if(usY + height > screen_height)
        height = screen_height - usY;
    
    if(width <= 0 || height <= 0)
    {
        BMP_DEBUG_PRINTF("Image size invalid!\r\n");
        f_close(&bmpfsrc);
        return 4;
    }
    
    /* 设置LCD扫描方向 */
    // LCD_SetScanDirection(1);
    
    /* 打开显示窗口 */
    LCD_SetWindow(usX, usY, width, height);
    LCD_COMMOUT(macCMD_SetPixel);
    
    /* ==================== 处理1位单色BMP ==================== */
    if(bitInfoHead.biBitCount == 1)
    {
        /* 读取调色板 */
        f_read(&bmpfsrc, palette, 8, &read_num);
        
        /* 计算BMP行字节数（4字节对齐）*/
        l_width = ((width + 31) / 32) * 4;
        
        if(l_width > 960)
        {
            BMP_DEBUG_PRINTF("Buffer too small!\r\n");
            f_close(&bmpfsrc);
            return 5;
        }
        
        /* 判断调色板颜色 */
        color0 = RGB24TORGB16(palette[2], palette[1], palette[0]);
        color1 = RGB24TORGB16(palette[6], palette[5], palette[4]);
        
        /* 一行一行读取并显示 */
        for(i = 0; i < height; i++)
        {
            f_lseek(&bmpfsrc, bitHead.bfOffBits + (height - i - 1) * l_width);
            f_read(&bmpfsrc, pColorData, l_width, &read_num);
            
            for(j = 0; j < width; j++)
            {
                byte_idx = j / 8;
                bit_idx = 7 - (j % 8);
                pixel = (pColorData[byte_idx] >> bit_idx) & 0x01;
                
                color_16bit = (pixel == 0) ? color0 : color1;
                LCD_Write_Data(color_16bit);
            }
        }
    }
    
    /* ==================== 处理24位真彩色BMP ==================== */
    else if(bitInfoHead.biBitCount == 24)
    {
        /* 计算BMP行字节数（4字节对齐）*/
        l_width = ((width * 24 + 31) / 32) * 4;
        
        if(l_width > 960)
        {
            BMP_DEBUG_PRINTF("Buffer too small! Need %d bytes for width %d\r\n", l_width, width);
            f_close(&bmpfsrc);
            return 5;
        }
        
        /* 一行一行读取并显示 */
        for(i = 0; i < height; i++)
        {
            f_lseek(&bmpfsrc, bitHead.bfOffBits + (height - i - 1) * l_width);
            f_read(&bmpfsrc, pColorData, l_width, &read_num);
            
            for(j = 0; j < width; j++)
            {
                k = j * 3;
                blue = pColorData[k + 2];
                green = pColorData[k + 1];
                red = pColorData[k];
                
                color_16bit = RGB24TORGB16(red, green, blue);
                LCD_Write_Data(color_16bit);
            }
        }
    }
    
    /* ==================== 处理16位彩色BMP ==================== */
    else if(bitInfoHead.biBitCount == 16)
    {
        /* 计算BMP行字节数（4字节对齐）*/
        l_width = ((width * 16 + 31) / 32) * 4;
        
        if(l_width > 960)
        {
            BMP_DEBUG_PRINTF("Buffer too small! Need %d bytes\r\n", l_width);
            f_close(&bmpfsrc);
            return 5;
        }
        
        /* 一行一行读取并显示 */
        for(i = 0; i < height; i++)
        {
            f_lseek(&bmpfsrc, bitHead.bfOffBits + (height - i - 1) * l_width);
            f_read(&bmpfsrc, pColorData, l_width, &read_num);
            
            for(j = 0; j < width; j++)
            {
                color_16bit = *(uint16_t*)(&pColorData[j * 2]);
                LCD_Write_Data(color_16bit);
            }
        }
    }
    
    else
    {
        BMP_DEBUG_PRINTF("Unsupported BMP format: %d-bit\r\n", bitInfoHead.biBitCount);
        f_close(&bmpfsrc);
        return 7;
    }
    
    f_close(&bmpfsrc);
    BMP_DEBUG_PRINTF("BMP display completed!\r\n");
    
    return 0;
}
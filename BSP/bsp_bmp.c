#include "bsp_bmp.h"
#include "ff.h"
#include "bsp_ili9341_lcd.h"
#include "FreeRTOS.h"     
#include "task.h"         

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
//tagRGBQUAD dataOfBmp[17*19];         /* 备用：BMP调色板数据缓冲区（已注释）*/
FATFS bmpfs[2];                         /* FATFS文件系统对象数组，用于SD卡文件系统 */
FIL bmpfsrc, bmpfdst;                   /* 文件对象：bmpfsrc用于读文件，bmpfdst用于写文件 */
FRESULT bmpres;                         /* FATFS操作结果码 */

/* 调试打印宏定义
 * 如需打印BMP相关的调试信息，请将下面的printf注释去掉
 * 注意：使用printf()需要包含相应的串口初始化代码
 */
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
	
	/* 计算文件总大小 = 像素数据 + 行填充字节 + 文件头
	 * 每个像素3字节(RGB)，每行需要填充到4的倍数
	 */
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
	
	/* 按指定格式组合完整文件名，保存到tmp_name */
	sprintf((char*)tmp_name, "0:%s.bmp", filename);
	
	/* 挂载SD卡文件系统（逻辑驱动器0）*/
	f_mount(0, &bmpfs[0]);
	
	/* 创建新文件（如果文件已存在则失败）*/
	bmpres = f_open( &bmpfsrc, (char*)tmp_name, FA_CREATE_NEW | FA_WRITE );
	
	/* 新创建的文件需要先关闭，再以读写方式重新打开 */
	f_close(&bmpfsrc);
	
	/* 以读写方式重新打开已存在的文件 */
	bmpres = f_open( &bmpfsrc, (char*)tmp_name, FA_OPEN_EXISTING | FA_WRITE );

	if ( bmpres == FR_OK )  // 文件打开成功
	{    
		/* 先将预定义的54字节BMP文件头写入文件 */
		bmpres = f_write(&bmpfsrc, header, sizeof(unsigned char) * 54, &mybw);		
		
		/* 设置LCD扫描方向为：左上角->右下角（正常显示方向）*/
		ILI9341_GramScan( 1 );
		
		/* 计算每行需要填充的字节数（使行字节数为4的倍数）*/
		ucAlign = Width % 4;
		
		/* 逐行读取LCD像素数据并写入文件 */
		for(i = 0; i < Height; i++)  // 遍历每一行
		{
			for(j = 0; j < Width; j++)  // 遍历当前行的每个像素
			{					
				/* 获取LCD指定坐标的像素颜色值（注意BMP存储顺序为从底向上）*/
				read_data = ILI9341_GetPointPixel ( x + j, y + Height - 1 - i );					
				
				/* 从RGB565格式中提取RGB分量（各5/6/5位）*/
				r = GETR_FROM_RGB16(read_data);  // 提取红色分量（5位，范围0-31）
				g = GETG_FROM_RGB16(read_data);  // 提取绿色分量（6位，范围0-63）
				b = GETB_FROM_RGB16(read_data);  // 提取蓝色分量（5位，范围0-31）

				/* BMP格式存储顺序为BGR，按顺序写入文件 */
				bmpres = f_write(&bmpfsrc, &b, sizeof(unsigned char), &mybw);  // 写入蓝色分量
				bmpres = f_write(&bmpfsrc, &g, sizeof(unsigned char), &mybw);  // 写入绿色分量
				bmpres = f_write(&bmpfsrc, &r, sizeof(unsigned char), &mybw);  // 写入红色分量
			}
			
			/* 如果行字节数不是4的倍数，需要填充0以达到4字节对齐 */
			if( ucAlign )  // 需要填充
				bmpres = f_write ( & bmpfsrc, kk, sizeof(unsigned char) * ( ucAlign ), & mybw );
		}  /* 行循环结束 */

		f_close(&bmpfsrc);  // 关闭文件
		
		return 0;  // 截图成功
	}
	else if ( bmpres == FR_EXIST )  // 文件已存在错误
		return FR_EXIST;	 			// 返回8
	
	else  // 其他错误
		return -1;  // 截图失败
}


/**
 * @brief  显示BMP图片（通过ILI9341驱动）
 * @param  x 在扫描模式1下显示位置的起始X坐标
 * @param  y 在扫描模式1下显示位置的起始Y坐标
 * @param  pic_name BMP文件的完整路径（如 "/11.bmp"）
 * @retval 无
 */
void Lcd_Show_bmp(uint16_t usX, uint16_t usY, const char *pPath)
{
	int i, j, k;                    // 循环变量：i-行计数，j-列计数，k-像素数据索引
	int width, height, l_width;     // width-图片宽度，height-图片高度，l_width-实际行字节数（4字节对齐）
	
	BYTE red, green, blue;          // RGB颜色分量
	BITMAPFILEHEADER bitHead;       // BMP文件头结构体
	BITMAPINFOHEADER bitInfoHead;   // BMP信息头结构体
	WORD fileType;                  // 文件类型标识（应为0x4D42即"BM"）
	
	unsigned int read_num;          // 实际读取的字节数
	unsigned char tmp_name[20];     // 临时文件名缓冲区
	sprintf((char*)tmp_name,"0:%s",pPath);  // 添加SD卡驱动号前缀"0:"，如"0:/11.bmp"
	
	f_mount(0, &bmpfs[0]);          // 挂载SD卡文件系统
	BMP_DEBUG_PRINTF("file mount ok \r\n");    
	
	/* 打开BMP文件 */
	bmpres = f_open( &bmpfsrc , (char *)tmp_name, FA_OPEN_EXISTING | FA_READ);
	
/*-------------------------------------------------------------------------------------------------------*/
	if(bmpres == FR_OK)  // 文件打开成功
	{
		BMP_DEBUG_PRINTF("Open file success\r\n");
		
		/* 读取文件头信息，前2个字节 */         
		f_read(&bmpfsrc, &fileType, sizeof(WORD), &read_num);     
		
		/* 判断是否为BMP文件，文件标识应为"BM"(0x4D42) */
		if(fileType != 0x4d42)
		{
			BMP_DEBUG_PRINTF("file is not .bmp file!\r\n");
			return;  // 不是BMP文件，退出
		}
		else
		{
			BMP_DEBUG_PRINTF("Ok this is .bmp file\r\n");	
		}        
		
		/* 读取BMP文件头信息（14字节）*/
		f_read(&bmpfsrc, &bitHead, sizeof(tagBITMAPFILEHEADER), &read_num);        
		showBmpHead(&bitHead);  // 调试输出文件头信息
		
		/* 读取位图信息头信息（40字节）*/
		f_read(&bmpfsrc, &bitInfoHead, sizeof(BITMAPINFOHEADER), &read_num);        
		showBmpInforHead(&bitInfoHead);  // 调试输出位图信息头
	}    
	else  // 文件打开失败
	{
		BMP_DEBUG_PRINTF("file open fail!\r\n");
		return;
	}    
/*-------------------------------------------------------------------------------------------------------*/
	/* 获取图片的宽度和高度 */
	width = bitInfoHead.biWidth;    // 图片宽度（像素）
	height = bitInfoHead.biHeight;  // 图片高度（像素）
	
	/* 计算图片实际行字节数，必须是4的倍数（Windows BMP格式要求）*/
	l_width = WIDTHBYTES(width * bitInfoHead.biBitCount);  // biBitCount: 每个像素的位数（24位真彩色为24）
	
	if(l_width > 960)  // 检查行缓冲区是否溢出（960字节对应320像素*24位）
	{
		BMP_DEBUG_PRINTF("\n SORRY, PIC IS TOO BIG (<=320)\n");
		return;
	}
	
	/* 设置LCD Gram扫描方向为: 左下角->右上角*/
//	ILI9341_GramScan (3);
	
	/* 在LCD上打开与图片大小相同的显示窗口 */
	ILI9341_OpenWindow(usX, usY, width, height);
	comm_out ( macCMD_SetPixel );  // 发送写像素命令，准备写入图片数据
	
	/* 判断是否为24位真彩色图片 */
    if(bitInfoHead.biBitCount == 1)  // 单色位图
    {
        // 读取调色板（2个颜色，共8字节）
        BYTE palette[8];  // 每个颜色4字节（B,G,R,保留）
        f_read(&bmpfsrc, palette, 8, &read_num);
        
        // 单色BMP的数据行字节数计算（每行需4字节对齐）
        l_width = ((width + 31) / 32) * 4;  // 每行字节数 = ceil(width/8)向上取整到4的倍数
        
        if(l_width > 960) {
            BMP_DEBUG_PRINTF("Picture too big!\r\n");
            return;
        }
        
        // 设置显示窗口
        ILI9341_OpenWindow(usX, usY, width, height);
        comm_out(macCMD_SetPixel);
        
        // 逐行处理（BMP存储是从底向上）
        for(i = 0; i < height; i++)
        {
            // 定位到当前行（BMP底部是第一行）
            f_lseek(&bmpfsrc, bitHead.bfOffBits + (height - i - 1) * l_width);
            
            // 读取一行数据
            f_read(&bmpfsrc, pColorData, l_width, &read_num);
            
            // 将单色数据转换为RGB565并显示
            for(j = 0; j < width; j++)
            {
                // 定位到当前像素所在的位
                int byte_idx = j / 8;      // 字节索引
                int bit_idx = 7 - (j % 8); // 位索引（BMP通常高位在前）
                
                // 获取该位的值（0或1）
                BYTE bit = (pColorData[byte_idx] >> bit_idx) & 0x01;
                
                // 根据位值选择颜色
                unsigned short color;
                if(bit == 1)  // 前景色（通常是白色）
                {
                    // 从调色板获取实际颜色，或直接使用白色
                    color = RGB24TORGB16(255, 255, 255);  // 白色
                }
                else  // 背景色（通常是黑色）
                {
                    color = RGB24TORGB16(0, 0, 0);  // 黑色
                }
                
                ILI9341_Write_Data(color);
            }
        }
    }else if( bitInfoHead.biBitCount >= 24 )  // 24位或32位真彩色
	{
		/* 逐行显示图片（从底向上，因为BMP存储顺序是从下到上）*/
		for ( i = 0; i < height; i ++ )
		{
			/* 移动文件指针到当前行的起始位置
			 * bfOffBits: 像素数据的起始偏移量
			 * (height - i - 1): BMP图片存储是倒序的，第一行对应图片最后一行
			 * l_width: 每行的字节数
			 */
			f_lseek ( & bmpfsrc, bitHead .bfOffBits + ( height - i - 1 ) * l_width );	
			
			/* 读取一行BMP数据到pColorData缓冲区 */
			#if 0
				/* 方法1：逐字节读取（效率最低）*/
				for(j=0; j< l_width; j++)	 													
				{                
					f_read(&bmpfsrc, pColorData+j, 1, &read_num);
				}            
				
			#elif 0	
				/* 方法2：分两次批量读取（效率中等）*/
				f_read ( & bmpfsrc, pColorData, l_width / 2, & read_num );		
				f_read ( & bmpfsrc, pColorData + l_width / 2, l_width / 2, & read_num );
				
			#else
				/* 方法3：一次性批量读取一行数据（效率最高，当前使用）*/
				f_read ( & bmpfsrc, pColorData, l_width, & read_num );
				
			#endif
			
			/* 将当前行的每个像素转换为RGB565格式并写入LCD */
			for(j = 0; j < width; j++)  // 遍历当前行的每个像素
			{
				k = j * 3;  // 计算当前像素在缓冲区中的索引（每个像素占3字节：B,G,R）
				blue = pColorData[k + 2];   // 蓝色分量（BMP存储顺序为BGR）
				green = pColorData[k + 1];  // 绿色分量
				red = pColorData[k];        // 红色分量
				
				/* 将24位RGB颜色转换为16位RGB565格式并写入LCD显存 */
				ILI9341_Write_Data ( RGB24TORGB16(red, green, blue));
				//ILI9341_Write_Data ( blue<<16|green<<8|red);  // 另一种格式（24位RGB，不常用）
			}            
		}        
	}    
	else  // 不是24位真彩色图片
	{        
		BMP_DEBUG_PRINTF("SORRY, THIS PIC IS NOT A 24BITS REAL COLOR");
		return ;
	}
	
	f_close(&bmpfsrc);  // 关闭BMP文件
}


/**
 * @brief  初始化序列帧播放器
 * @param  seq 序列帧控制结构体指针
 * @param  start 起始帧序号
 * @param  end 结束帧序号
 * @param  delay 帧延迟(ms)
 * @param  loop 是否循环(1:循环, 0:播放一次)
 * @retval 无
 */
void FrameSeq_Init(FrameSequence_t *seq, uint16_t start, uint16_t end, uint16_t delay, uint8_t loop)
{
    seq->start_index = start;
    seq->end_index = end;
    seq->current_frame = start;
    seq->delay_ms = delay;
    seq->loop = loop;
    seq->playing = 1;  // 默认启动播放
}

/**
 * @brief  设置序列帧文件路径
 * @param  seq 序列帧控制结构体指针
 * @param  prefix 路径前缀，如 "/" 或 "/animation/"
 * @param  format 文件格式，如 ".bmp"
 * @retval 无
 */
void FrameSeq_SetPath(FrameSequence_t *seq, const char *prefix, const char *format)
{
    strcpy(seq->path_prefix, prefix);
    strcpy(seq->file_format, format);
}

/**
 * @brief  播放序列帧
 * @param  seq 序列帧控制结构体指针
 * @param  x 显示X坐标
 * @param  y 显示Y坐标
 * @retval 无
 */
void FrameSeq_Play(FrameSequence_t *seq, uint16_t x, uint16_t y)
{
    seq->playing = 1;
    seq->current_frame = seq->start_index;
    
    while(seq->playing)
    {
        // 构建完整文件名
        char full_path[100];
        sprintf(full_path, "%s%d%s", seq->path_prefix, seq->current_frame, seq->file_format);
        
        // 显示当前帧
        Lcd_Show_bmp(x, y, full_path);
        
        // 延时
        vTaskDelay(pdMS_TO_TICKS(seq->delay_ms));
        
        // 更新到下一帧
        seq->current_frame++;
        
        // 检查是否播放完
        if(seq->current_frame > seq->end_index)
        {
            if(seq->loop)
            {
                seq->current_frame = seq->start_index;  // 循环播放
            }
            else
            {
                seq->playing = 0;  // 停止播放
                break;
            }
        }
    }
}

/**
 * @brief  停止序列帧播放
 * @param  seq 序列帧控制结构体指针
 * @retval 无
 */
void FrameSeq_Stop(FrameSequence_t *seq)
{
    seq->playing = 0;
}

/**
 * @brief  更新序列帧（非阻塞方式，需要在主循环或任务中调用）
 * @param  seq 序列帧控制结构体指针
 * @param  x 显示X坐标
 * @param  y 显示Y坐标
 * @retval 1:正在播放, 0:播放完成
 */
uint8_t FrameSeq_Update(FrameSequence_t *seq, uint16_t x, uint16_t y)
{
    static uint32_t last_time = 0;
    uint32_t current_time = xTaskGetTickCount();
    
    if(!seq->playing)
        return 0;
    
    // 检查是否需要更新下一帧
    if((current_time - last_time) >= pdMS_TO_TICKS(seq->delay_ms))
    {
        // 构建完整文件名
        char full_path[100];
        sprintf(full_path, "%s%d%s", seq->path_prefix, seq->current_frame, seq->file_format);
        
        // 显示当前帧
        Lcd_Show_bmp(x, y, full_path);
        
        // 更新到下一帧
        seq->current_frame++;
        
        // 检查是否播放完
        if(seq->current_frame > seq->end_index)
        {
            if(seq->loop)
            {
                seq->current_frame = seq->start_index;  // 循环播放
            }
            else
            {
                seq->playing = 0;  // 停止播放
                return 0;
            }
        }
        
        last_time = current_time;
    }
    
    return 1;
}

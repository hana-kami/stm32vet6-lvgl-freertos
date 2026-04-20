#include "bmp_browser.h"
#include "bsp_bmp.h"
#include "bsp_sdfs_app.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ff.h>

static DIR g_bmp_dir;
static uint16_t g_current_index = 0;
static uint16_t g_total_count = 0;
static char g_current_filename[64];
FATFS bmpfs1[2];                         /* FATFS文件系统对象数组，用于SD卡文件系统 */

/* 检查是否为BMP文件 */
static uint8_t is_bmp_file(const char* filename)
{
    const char* ext;
    int len = strlen(filename);
    
    if(len < 4) return 0;
    
    ext = filename + len - 4;
    return (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0);
}

/* 获取BMP文件总数（只计数，不存储） */
uint16_t BMP_Browser_GetTotalCount(void)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    uint16_t count = 0; 
    res = f_opendir(&dir, "/");
    if(res != FR_OK) return 0;
    
    while(1) {
        res = f_readdir(&dir, &fno);
        if(res != FR_OK || fno.fname[0] == 0) break;
        
        if(!(fno.fattrib & AM_DIR) && is_bmp_file(fno.fname)) {
            count++;
        }
    }
    
    return count;
}

/* 获取指定索引的BMP文件名（不存储，动态获取） */
static const char* get_bmp_filename_by_index(uint16_t index)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    uint16_t current_index = 0;
    
    res = f_opendir(&dir, "/");
    if(res != FR_OK) return NULL;
    
    while(1) {
        res = f_readdir(&dir, &fno);
        if(res != FR_OK || fno.fname[0] == 0) break;
        
        if(!(fno.fattrib & AM_DIR) && is_bmp_file(fno.fname)) {
            if(current_index == index) {
                strcpy(g_current_filename, fno.fname);
                return g_current_filename;
            }
            current_index++;
        }
    }
    
    return NULL;
}

/* 显示指定索引的BMP图片 */
uint8_t BMP_Browser_Show(uint16_t index, uint16_t x, uint16_t y)
{
    const char* filename;
    char full_path[100];
    filename = get_bmp_filename_by_index(index);
    if(filename == NULL) {
        printf("未找到索引 %d 的BMP文件\n", index);
        return 0;
    }
    
    sprintf(full_path, "/%s", filename);
    printf("显示图片[%d]: %s\n", index + 1, full_path);
    
    Lcd_Show_bmp(x, y, full_path);
    g_current_index = index;
    
    return 1;
}

/* 下一张图片 */
void BMP_Browser_Next(void)
{
    uint16_t total = BMP_Browser_GetTotalCount();
    if(total == 0) return;
    
    g_current_index++;
    if(g_current_index >= total) {
        g_current_index = 0;
    }
    
    BMP_Browser_Show(g_current_index, 0, 0);
}

/* 上一张图片 */
void BMP_Browser_Prev(void)
{
    uint16_t total = BMP_Browser_GetTotalCount();
    if(total == 0) return;
    
    if(g_current_index == 0) {
        g_current_index = total - 1;
    } else {
        g_current_index--;
    }
    
    BMP_Browser_Show(g_current_index, 0, 0);
}
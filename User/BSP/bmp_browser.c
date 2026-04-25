#include "bmp_browser.h"
#include "bsp_bmp.h"
#include "bsp_sdfs_app.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ff.h>

/* ==================== 静态变量 ==================== */
static uint16_t g_current_index = 0;
static uint16_t g_total_count = 0;
static uint8_t g_initialized = 0;
static char g_temp_filename[64];  // 临时缓冲区，只存一个文件名

/* 文件系统对象 */
static FATFS g_bmp_fs;

/* ==================== 内部函数 ==================== */

/**
 * @brief  检查是否为BMP文件
 */
static uint8_t is_bmp_file(const char* filename)
{
    const char* ext;
    int len = strlen(filename);
    
    if(len < 4) return 0;
    
    ext = filename + len - 4;
    return (strcmp(ext, ".bmp") == 0 || strcmp(ext, ".BMP") == 0);
}

/**
 * @brief  获取指定索引的文件名（不缓存，每次扫描）
 * @param  index 索引
 * @param  filename 输出缓冲区
 * @retval 1:成功, 0:失败
 */
static uint8_t get_filename_by_index(uint16_t index, char* filename)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    uint16_t current = 0;
    
    res = f_opendir(&dir, "/");
    if(res != FR_OK) return 0;
    
    while(1) {
        res = f_readdir(&dir, &fno);
        if(res != FR_OK || fno.fname[0] == 0) break;
        
        if(!(fno.fattrib & AM_DIR) && is_bmp_file(fno.fname)) {
            if(current == index) {
                strcpy(filename, fno.fname);
                return 1;
            }
            current++;
        }
    }
    return 0;
}

/**
 * @brief  获取文件总数
 */
static uint16_t get_total_count(void)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    uint16_t count = 0;
    uint16_t max_files = 30;  
    
    res = f_opendir(&dir, "/");
    if(res != FR_OK) return 0;
    
    while(count < max_files) {
        res = f_readdir(&dir, &fno);
        
        // 退出条件1：读取失败
        if(res != FR_OK) break;
        
        // 退出条件2：文件名空（目录结束）
        if(fno.fname[0] == 0) break;
        
        // 处理文件
        if(!(fno.fattrib & AM_DIR) && is_bmp_file(fno.fname)) {
            count++;
        }
    }
    
    return count;
}

/* ==================== 公共函数 ==================== */

/**
 * @brief  初始化BMP浏览器
 */
void BMP_Browser_Init(void)
{
    f_mount(0, &g_bmp_fs);
    g_total_count = get_total_count();
    g_current_index = 0;
    g_initialized = 1;
}

/**
 * @brief  获取文件总数
 */
uint16_t BMP_Browser_GetTotalCount(void)
{
    if(!g_initialized) {
        BMP_Browser_Init();
    }
    return g_total_count;
}

/**
 * @brief  显示当前图片
 */
uint8_t BMP_Browser_ShowCurrent(uint16_t x, uint16_t y)
{
    char full_path[64];
    
    if(!g_initialized) {
        BMP_Browser_Init();
    }
    
    if(g_total_count == 0) return 0;
    
    // 获取当前索引的文件名
    if(!get_filename_by_index(g_current_index, g_temp_filename)) {
        return 0;
    }
    
    sprintf(full_path, "/%s", g_temp_filename);
    Lcd_Show_bmp(x, y, full_path);
    
    return 1;
}

/**
 * @brief  下一张
 */
void BMP_Browser_Next(void)
{
    if(!g_initialized) {
        BMP_Browser_Init();
    }
    
    if(g_total_count == 0) return;
    
    g_current_index++;
    if(g_current_index >= g_total_count) {
        g_current_index = 0;
    }
    
    BMP_Browser_ShowCurrent(0, 0);
}

/**
 * @brief  上一张
 */
void BMP_Browser_Prev(void)
{
    if(!g_initialized) {
        BMP_Browser_Init();
    }
    
    if(g_total_count == 0) return;
    
    if(g_current_index == 0) {
        g_current_index = g_total_count - 1;
    } else {
        g_current_index--;
    }
    
    BMP_Browser_ShowCurrent(0, 0);
}

/**
 * @brief  跳转到指定索引
 */
void BMP_Browser_Goto(uint16_t index)
{
    if(!g_initialized) {
        BMP_Browser_Init();
    }
    
    if(index < g_total_count) {
        g_current_index = index;
        BMP_Browser_ShowCurrent(0, 0);
    }
}

/**
 * @brief  刷新（重新扫描文件数量）
 */
void BMP_Browser_Refresh(void)
{
    g_total_count = get_total_count();
    if(g_current_index >= g_total_count && g_total_count > 0) {
        g_current_index = 0;
    }
}

/**
 * @brief  获取当前索引
 */
uint16_t BMP_Browser_GetCurrentIndex(void)
{
    return g_current_index;
}
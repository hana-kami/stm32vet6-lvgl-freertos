#include "bsp_sdfs_app.h"
#include "bsp_sdio_sdcard.h"
#include "ff.h"
//#include "bsp_usart1.h"	
#include <stdio.h>


FATFS myfs[2];                 // Work area (file system object) for logical drive
FIL myfsrc, myfdst;            // file objects
FRESULT myres;                 // FatFs function common result code
BYTE mybuffer[512];            // file copy buffer
BYTE my_latest_buffer[512];
uint8_t mystring[512]="this is a MicroSD demo base on fatfs";
UINT mybr, mybw;               // File R/W count
int mya = 0;
char mypath[512]="0:";         // һ��Ҫ��ʼ��Ϊ0:


void Sd_fs_init(void)
{
    /* SD���жϳ�ʼ�� */
		SDIO_NVIC_Configuration();
		
		/* SD ��Ӳ����ʼ������ʼ���̷�Ϊ0 */	 
   f_mount(0,&myfs[0]);	       
}

void Sd_fs_test(void)
{
    int count=0;
		    
    SDIO_NVIC_Configuration();
    disk_initialize( 0 );	     /* SD ��Ӳ����ʼ�� */

    f_mount(0, &myfs[0]);
    myres = f_open( &myfsrc , "0:/my__Demo.TXT" ,/* FA_CREATE_NEW |*/ FA_WRITE);
    
    if ( myres == FR_OK )
    { 
        /* Write buffer to file */
        myres = f_write(&myfsrc, mystring, sizeof(mystring), &mybr);    
        f_close(&myfsrc);      
    }    
    else if ( myres == FR_EXIST )  //����ļ��Ѿ�����
    {	
    }    
    
    myres = f_open(&myfsrc, "0:/my__Demo.TXT", FA_OPEN_EXISTING | FA_READ); /* ���ļ� */	  //ok    
    mybr = 1;
    mya = 0;    
    
    for (;;) 
    {        
        for ( mya=0; mya<512; mya++ ) 	/* �建���� */
            mybuffer[mya]=0;
        
     	  myres = f_read( &myfsrc, mybuffer, sizeof(mybuffer), &mybr ); /* ���ļ���������ݶ��������� */
        sprintf( (char*)&my_latest_buffer[count*512], "%s" , mybuffer );	  //��ӡ��������        
        count++;							
        if (myres || mybr == 0) break;   // error or eof        	    	
    }    
    f_close(&myfsrc);	 /* �رմ򿪵��ļ� */    	    
}


/*******************************************************************************
* Function Name  : Sdfs_new
* Description    : �˽�һ���ļ���д������  
* Input          : new_file_name--�˽��ļ�������  
*				   write_buffer--д���ļ������ݻ�������ַ  
*				   buffer_size--��������С
* Output         : None
* Return         : 0(success)  1(file existed )  -1(fail)
* Attention		 : None
*******************************************************************************/           
int Sdfs_new(BYTE *new_file_name, BYTE *write_buffer, BYTE buffer_size)
{
    BYTE name_buffer[50];
    sprintf((char*)name_buffer,"0:%s",new_file_name);
    
    f_mount(0, &myfs[0]);
    myres = f_open( &myfsrc , (char*)name_buffer , FA_CREATE_NEW | FA_WRITE);
    
    if ( myres == FR_OK ) 
    { 
        
        myres = f_write(&myfsrc, write_buffer,buffer_size, &mybr); //д������   
        f_close(&myfsrc);
        
        return 0;      
    }
    
    else if ( myres == FR_EXIST )  //����ļ��Ѿ�����
    {
        return FR_EXIST;	 
    }
    
    else
    {
        return -1;
    }
    
}


/*******************************************************************************
* Function Name  : Sdfs_write
* Description    : ���ļ�д������  
* Input          : new_file_name--���������ļ�������  
*				   write_buffer--д���ļ������ݻ�������ַ  
*				   buffer_size--��������С
* Output         : None
* Return         : 0(success)   -1(fail)
* Attention		 : None
*******************************************************************************/            
int Sdfs_write(BYTE *write_file_name, BYTE *write_buffer, BYTE buffer_size)
{
    BYTE name_buffer[50];
    sprintf((char*)name_buffer,"0:%s",write_file_name);
    
    f_mount(0, &myfs[0]);
    myres = f_open( &myfsrc , (char*)name_buffer ,FA_WRITE);
    
    if ( myres == FR_OK )  
    { 
        /* Write buffer to file */	
        myres = f_write(&myfsrc, write_buffer,buffer_size, &mybr); //д������   
        f_close(&myfsrc);
        
        return 0;      
    }
    else 
	if(myres == FR_NO_FILE)	 //���û�и��ļ�
	{
        return FR_NO_FILE;
    } 
    else	
	return -1;
}



/*******************************************************************************
* Function Name  : Sdfs_read
* Description    : ��һ���ļ��������ݵ������� 
* Input          : read_file_name--�ļ�������  
*				   				save_buffer--������Ҫ����ĵ�ַ 
* Output         : None
* Return         : 0(success)  -1(fail)
* Attention		 : None
*******************************************************************************/ 
int Sdfs_read(BYTE *read_file_name, BYTE *save_buffer)
{
    
    int count=0;
    BYTE name_buffer[50];
    sprintf((char*)name_buffer,"0:%s",read_file_name);
    Sd_fs_init();
    f_mount(0, &myfs[0]);
    myres = f_open(&myfsrc , (char*)name_buffer , FA_OPEN_EXISTING | FA_READ);
    
    if ( myres == FR_OK ) 
    { 
        for (;;) 
        {
            
            for ( mya=0; mya<512; mya++ ) 	/* �建���� */
                mybuffer[mya]=0;
            
            myres = f_read( &myfsrc, mybuffer, sizeof(mybuffer), &mybr ); /* ���ļ������������512�ֽ�Ϊ��λ�������ػ����� */
            sprintf((char*)&save_buffer[count*512],"%s",mybuffer);					//��ӡ���û�ָ���Ļ�����������
            
            count++;							
            if (myres || mybr == 0) break;   // error or eof        	    	
        }  	
        
        return 0;
    }
    
    else
        return -1;
    
    
}

/*******************************************************************************
* Function Name  : GetGBKCode_from_sd
* Description    : ��SD���ֿ��ж�ȡ�������ݵ�ָ���Ļ����� 
* Input          : pBuffer---���ݱ����ַ  
*				   					c--�����ַ����ֽ��� 
* Output         : None
* Return         : 0(success)  -1(fail)
* Attention		 	 : None
*******************************************************************************/ 
//int GetGBKCode_from_sd(unsigned char* pBuffer,const unsigned char * c)
//{ 
//    unsigned char High8bit,Low8bit;
//    unsigned int pos;
//	
//    High8bit=*c;     /* ȡ��8λ���� */
//    Low8bit=*(c+1);  /* ȡ��8λ���� */
//	
////  printf("%d ,%d\r\n",High8bit,Low8bit);
////	printf("%x ,%x\r\n",High8bit,Low8bit);
//	
//    //pos = ((High8bit-0xb0)*94+Low8bit-0xa0-1)*2*16;
//    pos = ((High8bit-0xa0-16)*94+Low8bit-0xa0-1)*2*16;
//	
//    f_mount(0, &myfs[0]);
//    myres = f_open(&myfsrc , "0:/HZLIB.bin", FA_OPEN_EXISTING | FA_READ);
//    
//    if ( myres == FR_OK ) 
//    {
//        f_lseek (&myfsrc, pos);		//ָ��ƫ��
//        myres = f_read( &myfsrc, pBuffer, 32, &mybr );		 //16*16��С�ĺ��� ����ģ ռ��16*2���ֽ�
//        
//        f_close(&myfsrc);
//        
//        return 0;  
//    }    
//    else
//        return -1;    
//}
int GetGBKCode_from_sd ( uint8_t * pBuffer, uint16_t c)
{ 
    unsigned char High8bit,Low8bit;
    unsigned int pos;
	
    High8bit= c >> 8;     /* ȡ��8λ���� */
    Low8bit= c & 0x00FF;  /* ȡ��8λ���� */
	
//  printf("%d ,%d\r\n",High8bit,Low8bit);
//	printf("%x ,%x\r\n",High8bit,Low8bit);
	
    //pos = ((High8bit-0xb0)*94+Low8bit-0xa0-1)*2*16;
    pos = ((High8bit-0xa0-16)*94+Low8bit-0xa0-1)*2*16;
	
    f_mount(0, &myfs[0]);
    myres = f_open(&myfsrc , "0:/HZLIB.bin", FA_OPEN_EXISTING | FA_READ);
    
    if ( myres == FR_OK ) 
    {
        f_lseek (&myfsrc, pos);		//ָ��ƫ��
        myres = f_read( &myfsrc, pBuffer, 32, &mybr );		 //16*16��С�ĺ��� ����ģ ռ��16*2���ֽ�
        
        f_close(&myfsrc);
        
        return 0;  
    }    
    else
        return -1;    
}


#include "bsp_xpt2046_lcd.h"
#include "bsp_ili9341_lcd.h"
#include <stdio.h> 
#include <string.h>



/******************************* ���� XPT2046 ��صľ�̬���� ***************************/
static void                   XPT2046_EXTI_Config                   ( void );
static void                   XPT2046_EXTI_NVIC_Config              ( void );
static void                   XPT2046_GPIO_SPI_Config               ( void );

static void                   XPT2046_DelayUS                       ( __IO uint32_t ulCount );
static void                   XPT2046_WriteCMD                      ( uint8_t ucCmd );
static uint16_t               XPT2046_ReadCMD                       ( void );
static uint16_t               XPT2046_ReadAdc                       ( uint8_t ucChannel );
static void                   XPT2046_ReadAdc_XY                    ( int16_t * sX_Ad, int16_t * sY_Ad );
static uint8_t                XPT2046_ReadAdc_Smooth_XY             ( strType_XPT2046_Coordinate * pScreenCoordinate );
static uint8_t                XPT2046_Calculate_CalibrationFactor   ( strType_XPT2046_Coordinate * pDisplayCoordinate, strType_XPT2046_Coordinate * pScreenSample, strType_XPT2046_Calibration * pCalibrationFactor );
static void                   ILI9341_DrawCross                     ( uint16_t usX, uint16_t usY );



/******************************* ���� XPT2046 ȫ�ֱ��� ***************************/
strType_XPT2046_TouchPara strXPT2046_TouchPara = { 0.085958, -0.001073, -4.979353, -0.001750, 0.065168, -13.318824 };  //��ʼֵΪɨ�跽ʽ2�µ�һ��У׼ϵ������У׼������ʱ��ʹ�ø���Ĭ��ϵ��
                                              // { 0.001030, 0.064188, -10.804098, -0.085584, 0.001420, 324.127036 };  //��ʼֵΪɨ�跽ʽ1�µ�һ��У׼ϵ������У׼������ʱ��ʹ�ø���Ĭ��ϵ��

volatile uint8_t ucXPT2046_TouchFlag = 0;



/**
  * @brief  XPT2046 ��ʼ������
  * @param  ��
  * @retval ��
  */	
void XPT2046_Init ( void )
{
	XPT2046_GPIO_SPI_Config ();
	
	XPT2046_EXTI_Config ();
		
}


/**
  * @brief  ���� XPT2046 �ⲿ�ж����ȼ�
  * @param  ��
  * @retval ��
  */	
static void XPT2046_EXTI_NVIC_Config ( void )
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
	
//   /* Configure one bit for preemption priority */
//   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  /* �����ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = macXPT2046_EXTI_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
}


/**
  * @brief  ���� XPT2046 �ⲿ�ж�
  * @param  ��
  * @retval ��
  */	
static void XPT2046_EXTI_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	EXTI_InitTypeDef EXTI_InitStructure;

	
	/* config the extiline clock and AFIO clock */
	RCC_APB2PeriphClockCmd ( macXPT2046_EXTI_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE );
												
	/* config the NVIC */
	XPT2046_EXTI_NVIC_Config ();

	/* EXTI line gpio config*/	
  GPIO_InitStructure.GPIO_Pin = macXPT2046_EXTI_GPIO_PIN;       
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	 // ��������
  GPIO_Init(macXPT2046_EXTI_GPIO_PORT, &GPIO_InitStructure);

	/* EXTI line mode config */
  GPIO_EXTILineConfig(macXPT2046_EXTI_SOURCE_PORT, macXPT2046_EXTI_SOURCE_PIN); 
  EXTI_InitStructure.EXTI_Line = macXPT2046_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½����ж�
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	
  EXTI_Init(&EXTI_InitStructure); 
	
}


/**
  * @brief  ���� XPT2046 ��ģ��SPI
  * @param  ��
  * @retval ��
  */	
static void XPT2046_GPIO_SPI_Config ( void ) 
{ 
  GPIO_InitTypeDef  GPIO_InitStructure;
	

  /* ����GPIOʱ�� */
  RCC_APB2PeriphClockCmd ( macXPT2046_SPI_GPIO_CLK, ENABLE );

  /* ģ��SPI GPIO��ʼ�� */          
  GPIO_InitStructure.GPIO_Pin=macXPT2046_SPI_CLK_PIN;
  GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz ;	  
  GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
  GPIO_Init(macXPT2046_SPI_CLK_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_MOSI_PIN;
  GPIO_Init(macXPT2046_SPI_MOSI_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_MISO_PIN; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;      
  GPIO_Init(macXPT2046_SPI_MISO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = macXPT2046_SPI_CS_PIN; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      
  GPIO_Init(macXPT2046_SPI_CS_PORT, &GPIO_InitStructure); 
   
  /* ����Ƭѡ��ѡ��XPT2046 */
  macXPT2046_CS_DISABLE();

}




/**
  * @brief  ���� XPT2046 �ļ�΢�뼶��ʱ����
  * @param  nCount ����ʱ����ֵ����λΪ΢��
  * @retval ��
  */	
static void XPT2046_DelayUS ( __IO uint32_t ulCount )
{
	uint32_t i;


	for ( i = 0; i < ulCount; i ++ )
	{
		uint8_t uc = 12;     //����ֵΪ12����Լ��1΢��  
	      
		while ( uc -- );     //��1΢��	

	}
	
}



/**
  * @brief  XPT2046 ��д������
  * @param  ucCmd ������
  *   �ò���Ϊ����ֵ֮һ��
  *     @arg 0x90 :ͨ��Y+��ѡ�������
  *     @arg 0xd0 :ͨ��X+��ѡ�������
  * @retval ��
  */
static void XPT2046_WriteCMD ( uint8_t ucCmd ) 
{
	uint8_t i;


	macXPT2046_MOSI_0();
	
	macXPT2046_CLK_LOW();

	for ( i = 0; i < 8; i ++ ) 
	{
		( ( ucCmd >> ( 7 - i ) ) & 0x01 ) ? macXPT2046_MOSI_1() : macXPT2046_MOSI_0();
		
	  XPT2046_DelayUS ( 5 );
		
		macXPT2046_CLK_HIGH();

	  XPT2046_DelayUS ( 5 );

		macXPT2046_CLK_LOW();
	}
	
}


/**
  * @brief  XPT2046 �Ķ�ȡ����
  * @param  ��
  * @retval ��ȡ��������
  */
static uint16_t XPT2046_ReadCMD ( void ) 
{
	uint8_t i;
	uint16_t usBuf=0, usTemp;
	


	macXPT2046_MOSI_0();

	macXPT2046_CLK_HIGH();

	for ( i=0;i<12;i++ ) 
	{
		macXPT2046_CLK_LOW();    
	
		usTemp = macXPT2046_MISO();
		
		usBuf |= usTemp << ( 11 - i );
	
		macXPT2046_CLK_HIGH();
		
	}
	
	return usBuf;

}


/**
  * @brief  �� XPT2046 ѡ��һ��ģ��ͨ��������ADC��������ADC�������
  * @param  ucChannel
  *   �ò���Ϊ����ֵ֮һ��
  *     @arg 0x90 :ͨ��Y+��ѡ�������
  *     @arg 0xd0 :ͨ��X+��ѡ�������
  * @retval ��ͨ����ADC�������
  */
static uint16_t XPT2046_ReadAdc ( uint8_t ucChannel )
{
	XPT2046_WriteCMD ( ucChannel );

  return 	XPT2046_ReadCMD ();
	
}


/**
  * @brief  ��ȡ XPT2046 ��Xͨ����Yͨ����ADֵ��12 bit�������4096��
  * @param  sX_Ad �����Xͨ��ADֵ�ĵ�ַ
  * @param  sY_Ad �����Yͨ��ADֵ�ĵ�ַ
  * @retval ��
  */
static void XPT2046_ReadAdc_XY ( int16_t * sX_Ad, int16_t * sY_Ad )  
{ 
	int16_t sX_Ad_Temp, sY_Ad_Temp; 

	
	
	sX_Ad_Temp = XPT2046_ReadAdc ( macXPT2046_CHANNEL_X );

	XPT2046_DelayUS ( 1 ); 

	sY_Ad_Temp = XPT2046_ReadAdc ( macXPT2046_CHANNEL_Y ); 
	
	
	* sX_Ad = sX_Ad_Temp; 
	* sY_Ad = sY_Ad_Temp; 
	
	
}

 
/**
  * @brief  �ڴ��� XPT2046 ��Ļʱ��ȡһ�������ADֵ�����Ը���������˲�
  * @param  ��
  * @retval �˲�֮�������ADֵ
  */
#if   0                 //ע�⣺У���Ͼ�׼��������Ը��ӣ��ٶȽ���
static uint8_t XPT2046_ReadAdc_Smooth_XY ( strType_XPT2046_Coordinate * pScreenCoordinate )
{
	uint8_t ucCount = 0;
	
	int16_t sAD_X, sAD_Y;
	int16_t sBufferArray [ 2 ] [ 9 ] = { { 0 }, { 0 } };  //����X��Y����9�β���

	int32_t lAverage  [ 3 ], lDifference [ 3 ];
	

	do
	{		   
		XPT2046_ReadAdc_XY ( & sAD_X, & sAD_Y );
		
		sBufferArray [ 0 ] [ ucCount ] = sAD_X;  
		sBufferArray [ 1 ] [ ucCount ] = sAD_Y;
		
		ucCount ++; 
			 
	} while ( ( macXPT2046_EXTI_Read() == macXPT2046_EXTI_ActiveLevel ) && ( ucCount < 9 ) ); 	//�û����������ʱ��TP_INT_IN�ź�Ϊ�� ���� ucCount<9*/
	 
	
	/*������ʵ���*/
	if ( macXPT2046_EXTI_Read() != macXPT2046_EXTI_ActiveLevel )
		ucXPT2046_TouchFlag = 0;			//�����жϱ�־��λ		

	
	/* ����ɹ�����9��,�����˲� */ 
	if ( ucCount == 9 )   								
	{  
		/* Ϊ����������,�ֱ��3��ȡƽ��ֵ */
		lAverage  [ 0 ] = ( sBufferArray [ 0 ] [ 0 ] + sBufferArray [ 0 ] [ 1 ] + sBufferArray [ 0 ] [ 2 ] ) / 3;
		lAverage  [ 1 ] = ( sBufferArray [ 0 ] [ 3 ] + sBufferArray [ 0 ] [ 4 ] + sBufferArray [ 0 ] [ 5 ] ) / 3;
		lAverage  [ 2 ] = ( sBufferArray [ 0 ] [ 6 ] + sBufferArray [ 0 ] [ 7 ] + sBufferArray [ 0 ] [ 8 ] ) / 3;
		
		/* ����3�����ݵĲ�ֵ */
		lDifference [ 0 ] = lAverage  [ 0 ]-lAverage  [ 1 ];
		lDifference [ 1 ] = lAverage  [ 1 ]-lAverage  [ 2 ];
		lDifference [ 2 ] = lAverage  [ 2 ]-lAverage  [ 0 ];
		
		/* ��������ֵȡ����ֵ */
		lDifference [ 0 ] = lDifference [ 0 ]>0?lDifference [ 0 ]: ( -lDifference [ 0 ] );
		lDifference [ 1 ] = lDifference [ 1 ]>0?lDifference [ 1 ]: ( -lDifference [ 1 ] );
		lDifference [ 2 ] = lDifference [ 2 ]>0?lDifference [ 2 ]: ( -lDifference [ 2 ] );
		
		
		/* �жϾ��Բ�ֵ�Ƿ񶼳�����ֵ���ޣ������3�����Բ�ֵ����������ֵ�����ж���β�����ΪҰ��,���������㣬��ֵ����ȡΪ2 */
		if (  lDifference [ 0 ] > macXPT2046_THRESHOLD_CalDiff  &&  lDifference [ 1 ] > macXPT2046_THRESHOLD_CalDiff  &&  lDifference [ 2 ] > macXPT2046_THRESHOLD_CalDiff  ) 
			return 0;
		
		
		/* �������ǵ�ƽ��ֵ��ͬʱ��ֵ��strScreenCoordinate */ 
		if ( lDifference [ 0 ] < lDifference [ 1 ] )
		{
			if ( lDifference [ 2 ] < lDifference [ 0 ] ) 
				pScreenCoordinate ->x = ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
			else 
				pScreenCoordinate ->x = ( lAverage  [ 0 ] + lAverage  [ 1 ] ) / 2;	
		}
		
		else if ( lDifference [ 2 ] < lDifference [ 1 ] ) 
			pScreenCoordinate -> x = ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
		
		else 
			pScreenCoordinate ->x = ( lAverage  [ 1 ] + lAverage  [ 2 ] ) / 2;
		
		
		/* ͬ�ϣ�����Y��ƽ��ֵ */
		lAverage  [ 0 ] = ( sBufferArray [ 1 ] [ 0 ] + sBufferArray [ 1 ] [ 1 ] + sBufferArray [ 1 ] [ 2 ] ) / 3;
		lAverage  [ 1 ] = ( sBufferArray [ 1 ] [ 3 ] + sBufferArray [ 1 ] [ 4 ] + sBufferArray [ 1 ] [ 5 ] ) / 3;
		lAverage  [ 2 ] = ( sBufferArray [ 1 ] [ 6 ] + sBufferArray [ 1 ] [ 7 ] + sBufferArray [ 1 ] [ 8 ] ) / 3;
		
		lDifference [ 0 ] = lAverage  [ 0 ] - lAverage  [ 1 ];
		lDifference [ 1 ] = lAverage  [ 1 ] - lAverage  [ 2 ];
		lDifference [ 2 ] = lAverage  [ 2 ] - lAverage  [ 0 ];
		
		/* ȡ����ֵ */
		lDifference [ 0 ] = lDifference [ 0 ] > 0 ? lDifference [ 0 ] : ( - lDifference [ 0 ] );
		lDifference [ 1 ] = lDifference [ 1 ] > 0 ? lDifference [ 1 ] : ( - lDifference [ 1 ] );
		lDifference [ 2 ] = lDifference [ 2 ] > 0 ? lDifference [ 2 ] : ( - lDifference [ 2 ] );
		
		
		if ( lDifference [ 0 ] > macXPT2046_THRESHOLD_CalDiff && lDifference [ 1 ] > macXPT2046_THRESHOLD_CalDiff && lDifference [ 2 ] > macXPT2046_THRESHOLD_CalDiff ) 
			return 0;
		
		if ( lDifference [ 0 ] < lDifference [ 1 ] )
		{
			if ( lDifference [ 2 ] < lDifference [ 0 ] ) 
				pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
			else 
				pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 1 ] ) / 2;	
		}
		else if ( lDifference [ 2 ] < lDifference [ 1 ] ) 
			pScreenCoordinate ->y =  ( lAverage  [ 0 ] + lAverage  [ 2 ] ) / 2;
		else
			pScreenCoordinate ->y =  ( lAverage  [ 1 ] + lAverage  [ 2 ] ) / 2;
		
		
		return 1;
		
		
	}
	
	else if ( ucCount > 1 )
	{
		pScreenCoordinate ->x = sBufferArray [ 0 ] [ 0 ];
		pScreenCoordinate ->y = sBufferArray [ 1 ] [ 0 ];
	
		return 0;
		
	}  
	
	
	return 0; 
	
	
}


#else     //ע�⣺����Ӧ��ʵ��ר��,���Ǻܾ�׼�����Ǽ򵥣��ٶȱȽϿ�   
static uint8_t XPT2046_ReadAdc_Smooth_XY ( strType_XPT2046_Coordinate * pScreenCoordinate )
{
	uint8_t ucCount = 0, i;
	
	int16_t sAD_X, sAD_Y;
	int16_t sBufferArray [ 2 ] [ 10 ] = { { 0 },{ 0 } };  //����X��Y���ж�β���
	
	int32_t lX_Min, lX_Max, lY_Min, lY_Max;


	/* ѭ������10�� */ 
	do					       				
	{		  
		XPT2046_ReadAdc_XY ( & sAD_X, & sAD_Y );  
		
		sBufferArray [ 0 ] [ ucCount ] = sAD_X;  
		sBufferArray [ 1 ] [ ucCount ] = sAD_Y;
		
		ucCount ++;  
		
	}	while ( ( macXPT2046_EXTI_Read() == macXPT2046_EXTI_ActiveLevel ) && ( ucCount < 10 ) );//�û����������ʱ��TP_INT_IN�ź�Ϊ�� ���� ucCount<10
	
	
	/*������ʵ���*/
	if ( macXPT2046_EXTI_Read() != macXPT2046_EXTI_ActiveLevel )
		ucXPT2046_TouchFlag = 0;			//�жϱ�־��λ

	
	/*����ɹ�����10������*/
	if ( ucCount ==10 )		 					
	{
		lX_Max = lX_Min = sBufferArray [ 0 ] [ 0 ];
		lY_Max = lY_Min = sBufferArray [ 1 ] [ 0 ];       
		
		for ( i = 1; i < 10; i ++ )
		{
			if ( sBufferArray [ 0 ] [ i ] < lX_Min )
				lX_Min = sBufferArray [ 0 ] [ i ];
			
			else if ( sBufferArray [ 0 ] [ i ] > lX_Max )
				lX_Max = sBufferArray [ 0 ] [ i ];

		}
		
		for ( i = 1; i < 10; i ++ )
		{
			if ( sBufferArray [ 1 ] [ i ] < lY_Min )
				lY_Min = sBufferArray [ 1 ] [ i ];
			
			else if ( sBufferArray [ 1 ] [ i ] > lY_Max )
				lY_Max = sBufferArray [ 1 ] [ i ];

		}
		
		
		/*ȥ����Сֵ�����ֵ֮����ƽ��ֵ*/
		pScreenCoordinate ->x =  ( sBufferArray [ 0 ] [ 0 ] + sBufferArray [ 0 ] [ 1 ] + sBufferArray [ 0 ] [ 2 ] + sBufferArray [ 0 ] [ 3 ] + sBufferArray [ 0 ] [ 4 ] + 
		                           sBufferArray [ 0 ] [ 5 ] + sBufferArray [ 0 ] [ 6 ] + sBufferArray [ 0 ] [ 7 ] + sBufferArray [ 0 ] [ 8 ] + sBufferArray [ 0 ] [ 9 ] - lX_Min-lX_Max ) >> 3;
		
		pScreenCoordinate ->y =  ( sBufferArray [ 1 ] [ 0 ] + sBufferArray [ 1 ] [ 1 ] + sBufferArray [ 1 ] [ 2 ] + sBufferArray [ 1 ] [ 3 ] + sBufferArray [ 1 ] [ 4 ] + 
		                           sBufferArray [ 1 ] [ 5 ] + sBufferArray [ 1 ] [ 6 ] + sBufferArray [ 1 ] [ 7 ] + sBufferArray [ 1 ] [ 8 ] + sBufferArray [ 1 ] [ 9 ] - lY_Min-lY_Max ) >> 3; 
		
		
		return 1;
		
		
	}   
	
	
	return 0;    
	
	
}


#endif


/**
  * @brief  ���� XPT2046 ��������У��ϵ����ע�⣺ֻ����LCD�ʹ�����������Ƕȷǳ�Сʱ,�����������湫ʽ��
  * @param  pDisplayCoordinate ����Ļ��Ϊ��ʾ����֪����
  * @param  pstrScreenSample ������֪����㴥��ʱ XPT2046 ����������
  * @param  pCalibrationFactor ��������Ϊ�趨����Ͳ�����������������������Ļ����У��ϵ��
  * @retval ����״̬
	*   �÷���ֵΪ����ֵ֮һ��
  *     @arg 1 :����ɹ�
  *     @arg 0 :����ʧ��
  */
static uint8_t XPT2046_Calculate_CalibrationFactor ( strType_XPT2046_Coordinate * pDisplayCoordinate, strType_XPT2046_Coordinate * pScreenSample, strType_XPT2046_Calibration * pCalibrationFactor )
{
	uint8_t ucRet = 1;

	
	/* K�� ( X0��X2 )  ( Y1��Y2 )�� ( X1��X2 )  ( Y0��Y2 ) */
	pCalibrationFactor -> Divider =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
									                 ( ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) ) ;
	
	
	if (  pCalibrationFactor -> Divider == 0  )
		ucRet = 0;

	else
	{
		/* A�� (  ( XD0��XD2 )  ( Y1��Y2 )�� ( XD1��XD2 )  ( Y0��Y2 ) )��K	*/
		pCalibrationFactor -> An =  ( ( pDisplayCoordinate [ 0 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 1 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) );
		
		/* B�� (  ( X0��X2 )  ( XD1��XD2 )�� ( XD0��XD2 )  ( X1��X2 ) )��K	*/
		pCalibrationFactor -> Bn =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pDisplayCoordinate [ 1 ] .x - pDisplayCoordinate [ 2 ] .x ) ) - 
								                ( ( pDisplayCoordinate [ 0 ] .x - pDisplayCoordinate [ 2 ] .x ) *  ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) );
		
		/* C�� ( Y0 ( X2XD1��X1XD2 )+Y1 ( X0XD2��X2XD0 )+Y2 ( X1XD0��X0XD1 ) )��K */
		pCalibrationFactor -> Cn =  ( pScreenSample [ 2 ] .x * pDisplayCoordinate [ 1 ] .x - pScreenSample [ 1 ] .x * pDisplayCoordinate [ 2 ] .x ) * pScreenSample [ 0 ] .y +
								                ( pScreenSample [ 0 ] .x * pDisplayCoordinate [ 2 ] .x - pScreenSample [ 2 ] .x * pDisplayCoordinate [ 0 ] .x ) * pScreenSample [ 1 ] .y +
								                ( pScreenSample [ 1 ] .x * pDisplayCoordinate [ 0 ] .x - pScreenSample [ 0 ] .x * pDisplayCoordinate [ 1 ] .x ) * pScreenSample [ 2 ] .y ;
		
		/* D�� (  ( YD0��YD2 )  ( Y1��Y2 )�� ( YD1��YD2 )  ( Y0��Y2 ) )��K	*/
		pCalibrationFactor -> Dn =  ( ( pDisplayCoordinate [ 0 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 1 ] .y - pScreenSample [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 1 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 0 ] .y - pScreenSample [ 2 ] .y ) ) ;
		
		/* E�� (  ( X0��X2 )  ( YD1��YD2 )�� ( YD0��YD2 )  ( X1��X2 ) )��K	*/
		pCalibrationFactor -> En =  ( ( pScreenSample [ 0 ] .x - pScreenSample [ 2 ] .x ) *  ( pDisplayCoordinate [ 1 ] .y - pDisplayCoordinate [ 2 ] .y ) ) - 
								                ( ( pDisplayCoordinate [ 0 ] .y - pDisplayCoordinate [ 2 ] .y ) *  ( pScreenSample [ 1 ] .x - pScreenSample [ 2 ] .x ) ) ;
		
		
		/* F�� ( Y0 ( X2YD1��X1YD2 )+Y1 ( X0YD2��X2YD0 )+Y2 ( X1YD0��X0YD1 ) )��K */
		pCalibrationFactor -> Fn =  ( pScreenSample [ 2 ] .x * pDisplayCoordinate [ 1 ] .y - pScreenSample [ 1 ] .x * pDisplayCoordinate [ 2 ] .y ) * pScreenSample [ 0 ] .y +
								                ( pScreenSample [ 0 ] .x * pDisplayCoordinate [ 2 ] .y - pScreenSample [ 2 ] .x * pDisplayCoordinate [ 0 ] .y ) * pScreenSample [ 1 ] .y +
								                ( pScreenSample [ 1 ] .x * pDisplayCoordinate [ 0 ] .y - pScreenSample [ 0 ] .x * pDisplayCoordinate [ 1 ] .y ) * pScreenSample [ 2 ] .y;
			
	}
	
	
	return ucRet;
	
	
}


/**
  * @brief  �� ILI9341 ����ʾУ������ʱ��Ҫ��ʮ��
  * @param  usX �����ض�ɨ�跽����ʮ�ֽ�����X����
  * @param  usY �����ض�ɨ�跽����ʮ�ֽ�����Y����
  * @retval ��
  */
static void ILI9341_DrawCross ( uint16_t usX, uint16_t usY )
{
  ILI9341_Clear ( usX - 10, usY, 20, 1, macRED);
  ILI9341_Clear ( usX, usY - 10, 1, 20, macRED);
	
}


/**
  * @brief  XPT2046 ������У׼
  * @param  ��
  * @retval У׼���
	*   �÷���ֵΪ����ֵ֮һ��
  *     @arg 1 :У׼�ɹ�
  *     @arg 0 :У׼ʧ��
  */
uint8_t XPT2046_Touch_Calibrate ( void )
{
	#if 1
		uint8_t i;
		
		char cStr [ 10 ];
		
    uint16_t usScreenWidth, usScreenHeigth;
		uint16_t usTest_x = 0, usTest_y = 0, usGap_x = 0, usGap_y = 0;
		
	  char * pStr = 0;
	
    strType_XPT2046_Coordinate strCrossCoordinate [ 4 ], strScreenSample [ 4 ];
	  
	  strType_XPT2046_Calibration CalibrationFactor;
    		

		#if ( macXPT2046_Coordinate_GramScan == 1 ) || ( macXPT2046_Coordinate_GramScan == 4 )
	    usScreenWidth = macILI9341_Default_Max_Width;
	    usScreenHeigth = macILI9341_Default_Max_Heigth;

	  #elif ( macXPT2046_Coordinate_GramScan == 2 ) || ( macXPT2046_Coordinate_GramScan == 3 )
	    usScreenWidth = macILI9341_Default_Max_Heigth;
	    usScreenHeigth = macILI9341_Default_Max_Width;
	
	  #endif
		
		
		/* �趨��ʮ���ֽ��������� */ 
		strCrossCoordinate [ 0 ] .x = usScreenWidth >> 2;
		strCrossCoordinate [ 0 ] .y = usScreenHeigth >> 2;
		
		strCrossCoordinate [ 1 ] .x = strCrossCoordinate [ 0 ] .x;
		strCrossCoordinate [ 1 ] .y = ( usScreenHeigth * 3 ) >> 2;
		
		strCrossCoordinate [ 2 ] .x = ( usScreenWidth * 3 ) >> 2;
		strCrossCoordinate [ 2 ] .y = strCrossCoordinate [ 1 ] .y;
		
		strCrossCoordinate [ 3 ] .x = strCrossCoordinate [ 2 ] .x;
		strCrossCoordinate [ 3 ] .y = strCrossCoordinate [ 0 ] .y;		
	  	
			
		ILI9341_GramScan ( macXPT2046_Coordinate_GramScan );
		
		
		for ( i = 0; i < 4; i ++ )
		{ 
			ILI9341_Clear ( 0, 0, usScreenWidth, usScreenHeigth, macBACKGROUND );       
			
			pStr = "Touch Calibrate ......";			
      ILI9341_DispString_EN ( ( usScreenWidth - ( strlen ( pStr ) - 7 ) * macWIDTH_EN_CHAR ) >> 1, usScreenHeigth >> 1, pStr, macBACKGROUND, macRED );			
		
			sprintf ( cStr, "%d", i + 1 );
			ILI9341_DispString_EN ( usScreenWidth >> 1, ( usScreenHeigth >> 1 ) - macHEIGHT_EN_CHAR, cStr, macBACKGROUND, macRED );
		
			XPT2046_DelayUS ( 100000 );		                                                   //�ʵ�����ʱ���б�Ҫ
			
			ILI9341_DrawCross ( strCrossCoordinate [ i ] .x, strCrossCoordinate [ i ] .y );  //��ʾУ���õġ�ʮ����

			while ( ! XPT2046_ReadAdc_Smooth_XY ( & strScreenSample [ i ] ) );               //��ȡXPT2046���ݵ�����pCoordinate����ptrΪ��ʱ��ʾû�д��㱻����

		}
		
		
		XPT2046_Calculate_CalibrationFactor ( strCrossCoordinate, strScreenSample, & CalibrationFactor ) ;  	 //��ԭʼ��������� ԭʼ�����������ת��ϵ��
		
		if ( CalibrationFactor .Divider == 0 ) goto Failure;
		
			
		usTest_x = ( ( CalibrationFactor .An * strScreenSample [ 3 ] .x ) + ( CalibrationFactor .Bn * strScreenSample [ 3 ] .y ) + CalibrationFactor .Cn ) / CalibrationFactor .Divider;		//ȡһ�������Xֵ	 
		usTest_y = ( ( CalibrationFactor .Dn * strScreenSample [ 3 ] .x ) + ( CalibrationFactor .En * strScreenSample [ 3 ] .y ) + CalibrationFactor .Fn ) / CalibrationFactor .Divider;    //ȡһ�������Yֵ
		
		usGap_x = ( usTest_x > strCrossCoordinate [ 3 ] .x ) ? ( usTest_x - strCrossCoordinate [ 3 ] .x ) : ( strCrossCoordinate [ 3 ] .x - usTest_x );   //ʵ��X�������������ľ��Բ�
		usGap_y = ( usTest_y > strCrossCoordinate [ 3 ] .y ) ? ( usTest_y - strCrossCoordinate [ 3 ] .y ) : ( strCrossCoordinate [ 3 ] .y - usTest_y );   //ʵ��Y�������������ľ��Բ�
		
    if ( ( usGap_x > 10 ) || ( usGap_y > 10 ) ) goto Failure;       //����ͨ���޸�������ֵ�Ĵ�С����������    
		

    /* У׼ϵ��Ϊȫ�ֱ��� */ 
		strXPT2046_TouchPara .dX_X = ( CalibrationFactor .An * 1.0 ) / CalibrationFactor .Divider;
		strXPT2046_TouchPara .dX_Y = ( CalibrationFactor .Bn * 1.0 ) / CalibrationFactor .Divider;
		strXPT2046_TouchPara .dX   = ( CalibrationFactor .Cn * 1.0 ) / CalibrationFactor .Divider;
		
		strXPT2046_TouchPara .dY_X = ( CalibrationFactor .Dn * 1.0 ) / CalibrationFactor .Divider;
		strXPT2046_TouchPara .dY_Y = ( CalibrationFactor .En * 1.0 ) / CalibrationFactor .Divider;
		strXPT2046_TouchPara .dY   = ( CalibrationFactor .Fn * 1.0 ) / CalibrationFactor .Divider;

//    /* ��ӡУУ׼ϵ�� */ 
//		printf ( "У׼ϵ�����£�\r\n" );
//		
//    for ( i = 0; i < 6; i ++ )
//		{
//			uint32_t ulHeadAddres = ( uint32_t ) ( & strXPT2046_TouchPara );
//			
//			
//			printf ( "%LF\r\n", * ( ( long double * ) ( ulHeadAddres + sizeof ( long double ) * i ) ) );
//			
//		}	
		

	#endif
	
	
	ILI9341_Clear ( 0, 0, usScreenWidth, usScreenHeigth, macBACKGROUND );
	
	pStr = "Calibrate Succed";			
	ILI9341_DispString_EN ( ( usScreenWidth - strlen ( pStr ) * macWIDTH_EN_CHAR ) >> 1, usScreenHeigth >> 1, pStr, macBACKGROUND, macRED );	

  XPT2046_DelayUS ( 200000 );

	return 1;    
	

	Failure:
	
	ILI9341_Clear ( 0, 0, usScreenWidth, usScreenHeigth, macBACKGROUND ); 
	
	pStr = "Calibrate fail";			
	ILI9341_DispString_EN ( ( usScreenWidth - strlen ( pStr ) * macWIDTH_EN_CHAR ) >> 1, usScreenHeigth >> 1, pStr, macBACKGROUND, macRED );	

	pStr = "try again";			
	ILI9341_DispString_EN ( ( usScreenWidth - strlen ( pStr ) * macWIDTH_EN_CHAR ) >> 1, ( usScreenHeigth >> 1 ) + macHEIGHT_EN_CHAR, pStr, macBACKGROUND, macRED );				

	XPT2046_DelayUS ( 1000000 );		
	
	return 0; 
		
		
}

   
/**
  * @brief  ��ȡ XPT2046 �����㣨У׼�󣩵�����
  * @param  pDisplayCoordinate ����ָ���Ż�ȡ���Ĵ���������
  * @param  pTouchPara������У׼ϵ��
  * @retval ��ȡ���
	*   �÷���ֵΪ����ֵ֮һ��
  *     @arg 1 :��ȡ�ɹ�
  *     @arg 0 :��ȡʧ��
  */
uint8_t XPT2046_Get_TouchedPoint ( strType_XPT2046_Coordinate * pDisplayCoordinate, strType_XPT2046_TouchPara * pTouchPara )
{
	uint8_t ucRet = 1;           //���������򷵻�0
	
	strType_XPT2046_Coordinate strScreenCoordinate; 
	

  if ( XPT2046_ReadAdc_Smooth_XY ( & strScreenCoordinate ) )
  {    
		pDisplayCoordinate ->x = ( ( pTouchPara ->dX_X * strScreenCoordinate .x ) + ( pTouchPara ->dX_Y * strScreenCoordinate .y ) + pTouchPara ->dX );        
		pDisplayCoordinate ->y = ( ( pTouchPara ->dY_X * strScreenCoordinate .x ) + ( pTouchPara ->dY_Y * strScreenCoordinate .y ) + pTouchPara ->dY );

  }
	 
	else ucRet = 0;            //�����ȡ�Ĵ�����Ϣ�����򷵻�0
	
	
	
  return ucRet;
	
	
} 

















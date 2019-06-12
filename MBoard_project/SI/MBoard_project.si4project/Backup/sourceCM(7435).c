#include "sourceCM.h"    //智能电表驱动进程函数；

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明
//extern datsTransCMD_FLAG1;
	
//默认RS485 9600B

osThreadId tid_sourceCM_Thread;
osThreadDef(sourceCM_Thread,osPriorityNormal,1,512);
			 
osPoolId  sourceCM_pool;								 
osPoolDef(sourceCM_pool, 10, sourceCM_MEAS);                  // 内存池定义
osMessageQId  MsgBox_sourceCM;
osMessageQDef(MsgBox_sourceCM, 2, &sourceCM_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTsourceCM;
osMessageQDef(MsgBox_MTsourceCM, 2, &sourceCM_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPsourceCM;
osMessageQDef(MsgBox_DPsourceCM, 2, &sourceCM_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void sourceCM_ioInit(void){//端口初始化

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );	                

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	
}

void sourceCM_Init(void){
	uart_init(9600);
	//sourceCM_ioInit();
	//sourceCM_ADCInit();
}

unsigned char ADD_CHECK_8b(u8 dats [ ], u8 length)
{
	u8 loop;
	u8	result = 0;
	
	for(loop = 0;loop < length;loop ++){
	
		result += dats[loop];
	}
	return result;
}

unsigned char empty_id(unsigned char *id)
{
	char i = 0;
	for ( ; i  < 6; i++)
		{
			if(id[i] != 0xAA)
				return 0;
		}
	return 1;
}

unsigned char read_data2id(unsigned char *data,unsigned char *id,unsigned char len)
{
	unsigned char i = 4;
	unsigned char add = 0;
	for( ; i  < len-2; i++)//data[4]->data[len-2]的和校验
		{
			if(data[4] != 0x68 || data[10] != 0x68 || data[len-1] != 0x16 )
				return 0;
			add += data[len];
			
		}
	if(add == data[len-2])
	{
		for ( i = 0; i  < 6; i++)
		{
			id[i] = data[i+5];
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

void sourceCM_Thread(const void *argument){

	osEvent  evt;
    osStatus status;
	unsigned char temp_id[6] 	= {0xAA ,0xAA ,0xAA ,0xAA ,0xAA ,0xAA };
	unsigned char read_id[12] 	= {0x68 ,0xAA ,0xAA ,0xAA ,0xAA ,0xAA ,0xAA ,0x68 ,0x13 ,0x00 ,0xDF ,0x16};
	unsigned char power_kwh[4]	= {0x33,0x33,0x34,0x33};//电能指令，单位kwh,其地址码每位都是需要加0x33
	unsigned char power_v[4]	= {0x33,0x32,0x34,0x35};//电压指令，单位V
	unsigned char power_a[4] 	= {0x33,0x32,0x35,0x35};//电流指令，单位A

	unsigned char send_buff[24]	= {0x68 ,
									0,0,0,0,0,0,
									0x68,
									0x11,0x04,
									0x00,0x00,0x00,0x00,
									0x00,
									0x16};
	
	const bool UPLOAD_MODE = false;	//1：数据变化时才上传 0：周期定时上传
	
	const uint8_t upldPeriod = 5;	//数据上传周期因数（UPLOAD_MODE = false 时有效）
	
	static char flag=0;
	uint8_t UPLDcnt = 0;
	bool UPLD_EN = false;
	
	const uint8_t dpSize = 40;
	const uint8_t dpPeriod = 10;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];

	sourceCM_MEAS actuatorData;	//本地输入量
	static sourceCM_MEAS Data_temp   = {1};	//下行数据输入量同步对比缓存
	static sourceCM_MEAS Data_tempDP = {1};	//本地输入量显示数据对比缓存
	
	sourceCM_MEAS *mptr = NULL;
	sourceCM_MEAS *rptr = NULL;
	
	for(;;){
		
		evt = osMessageGet(MsgBox_MTsourceCM, 100);
		if (evt.status == osEventMessage){		//等待消息指令
			
				rptr = evt.value.p;
				/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
				if (rptr->source_addr == datsTransCMD_FLAG1)
				{
					actuatorData.Switch	= rptr->Switch;
					//actuatorData.anaVal	= rptr->anaVal;
					flag=1;

					Data_temp.Switch = actuatorData.Switch;		//缓存区数据同步，避免下行数据回发
					//Data_temp.anaVal = actuatorData.anaVal;
				}

				do{status = osPoolFree(sourceCM_pool, rptr);}while(status != osOK);	//内存释放
				rptr = NULL;
				
			
				
				/*执行完后立即发一次状态*/
				do{mptr = (sourceCM_MEAS *)osPoolCAlloc(sourceCM_pool);}while(mptr == NULL);	//无线数据传输消息推送
				mptr->source_addr = datsTransCMD_FLAG1;
				mptr->Switch = actuatorData.Switch;
				mptr->anaVal = actuatorData.anaVal;
				osMessagePut(MsgBox_sourceCM, (uint32_t)mptr, 50);
		}	
		
		actuatorData.anaVal = 4;
		
		if(!UPLOAD_MODE){	//选择上传触发模式
		
				if(UPLDcnt < upldPeriod)UPLDcnt ++;
				else{
				
					UPLDcnt = 0;
					UPLD_EN = true;
				}
		}
		else
		{
		
			if(Data_temp.power_kwh != actuatorData.power_kwh){	
			
					Data_temp.power_kwh = actuatorData.power_kwh;
					UPLD_EN = true;
				}
		}
		
		if(UPLD_EN){
			
				UPLD_EN = false;
				
				do{mptr = (sourceCM_MEAS *)osPoolCAlloc(sourceCM_pool);}while(mptr == NULL);	//无线数据传输消息推送
				if(flag==1){mptr->source_addr = datsTransCMD_FLAG1;}
				mptr->power_kwh = actuatorData.power_kwh;
				mptr->power_A = actuatorData.power_A;
				mptr->power_v = actuatorData.power_v;
				osMessagePut(MsgBox_sourceCM, (uint32_t)mptr, 100);
		}
		
		if(Data_tempDP.power_kwh != actuatorData.power_kwh 
			|| Data_tempDP.power_A != actuatorData.power_A 
			|| Data_tempDP.power_v != actuatorData.power_v){
		
				Data_tempDP.power_kwh	= actuatorData.power_kwh; 
				Data_tempDP.power_A		= actuatorData.power_A ;
				Data_tempDP.power_v 	= actuatorData.power_v;
		   
				do{mptr = (sourceCM_MEAS *)osPoolCAlloc(sourceCM_pool);}while(mptr == NULL);	//1.44寸液晶显示消息推送
				if(flag==1){mptr->source_addr = datsTransCMD_FLAG1;}
				mptr->power_kwh	= actuatorData.power_kwh; 
				mptr->power_A	= actuatorData.power_A ;
				mptr->power_v 	= actuatorData.power_v;
				osMessagePut(MsgBox_DPsourceCM, (uint32_t)mptr, 100);
				osDelay(10);
		   }
	
		if(Pcnt < dpPeriod)
			{
				osDelay(10);Pcnt ++;
		}
		else{
		
				Pcnt = 0;
				
				osDelay(20);
		}
		
		osDelay(10);
		//未获取ID
		if( empty_id(temp_id) == 1  )
		{
			RS485_Send_Data(read_id, 12);
			delay_ms(10);
			if(RS485_RX_CNT != 0)
			{
				read_data2id(RS485_RX_BUF, temp_id, RS485_RX_CNT);
				RS485_RX_CNT = 0;
			}

			memcpy(send_buff+1, temp_id , 6);//给ID到发送缓存
		}
		else//以获取ID
		{
			//获取电能
			memcpy(send_buff+10, power_kwh , 4);
			send_buff[14] = ADD_CHECK_8b(send_buff,15);
			RS485_Send_Data(read_id, 12);
			delay_ms(10);
			if(RS485_RX_CNT > 0 && RS485_RX_BUF[RS485_RX_CNT - 2] == ADD_CHECK_8b(RS485_RX_BUF+4,RS485_RX_CNT - 2 -4))
			{
				actuatorData.power_kwh = 
					((RS485_RX_BUF[18]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f) + 
					((RS485_RX_BUF[19]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*10 +
					((RS485_RX_BUF[20]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*100+
					((RS485_RX_BUF[21]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*1000;
				RS485_RX_CNT = 0;
				
			}
			//获取电压
			memcpy(send_buff+10, power_v , 4);
			send_buff[14] = ADD_CHECK_8b(send_buff,15);
			RS485_Send_Data(send_buff, 16);
			delay_ms(10);
			if(RS485_RX_CNT > 0 && RS485_RX_BUF[RS485_RX_CNT - 2] == ADD_CHECK_8b(RS485_RX_BUF+4,RS485_RX_CNT - 2 -4))
			{
				actuatorData.power_v = 
					((RS485_RX_BUF[18]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f) + 
					((RS485_RX_BUF[19]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*10 +
					((RS485_RX_BUF[20]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*100+
					((RS485_RX_BUF[21]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*1000+
					((RS485_RX_BUF[22]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*1000+
					((RS485_RX_BUF[23]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*10000;
			}
			//获取电流
			memcpy(send_buff+10, power_a , 4);
			send_buff[14] = ADD_CHECK_8b(send_buff,15);
			RS485_Send_Data(send_buff, 16);
			delay_ms(10);
			if(RS485_RX_CNT > 0 && RS485_RX_BUF[RS485_RX_CNT - 2] == ADD_CHECK_8b(RS485_RX_BUF+4,RS485_RX_CNT - 2 -4))
			{
				actuatorData.power_A = 
					((RS485_RX_BUF[18]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f) + 
					((RS485_RX_BUF[19]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*10 +
					((RS485_RX_BUF[20]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*100+
					((RS485_RX_BUF[21]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*1000+
					((RS485_RX_BUF[22]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*1000+
					((RS485_RX_BUF[23]-0x33)>>4 *10 + (RS485_RX_BUF[18]-0x33)&0x0f)*10000;
				RS485_RX_CNT = 0;
			}
		}
		
	}
}

void sourceCMThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		sourceCM_pool   = osPoolCreate(osPool(sourceCM_pool));	//创建内存池
		MsgBox_sourceCM = osMessageCreate(osMessageQ(MsgBox_sourceCM), NULL);   //创建消息队列
		MsgBox_MTsourceCM = osMessageCreate(osMessageQ(MsgBox_MTsourceCM), NULL);//创建消息队列
		MsgBox_DPsourceCM = osMessageCreate(osMessageQ(MsgBox_DPsourceCM), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	sourceCM_Init();
	tid_sourceCM_Thread = osThreadCreate(osThread(sourceCM_Thread),NULL);
}

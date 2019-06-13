#include "smokeMS.h"//�۳�����������̺�����

extern ARM_DRIVER_USART Driver_USART1;		//�豸�����⴮��һ�豸����

osThreadId tid_smokeMS_Thread;
osThreadId tid_smokeMS_led_Thread;

static uint8_t temp;

osThreadDef(smokeMS_Thread,osPriorityNormal,1,512);
osThreadDef(smokeMS_led_Thread,osPriorityNormal,1,512);

osPoolId  smokeMS_pool;								 
osPoolDef(smokeMS_pool, 10, smokeMS_MEAS);                  // �ڴ�ض���
osMessageQId  MsgBox_smokeMS;
osMessageQDef(MsgBox_smokeMS, 2, &smokeMS_MEAS);            // ��Ϣ���ж��壬����ģ���߳�������ͨѶ�߳�
osMessageQId  MsgBox_MTsmokeMS;
osMessageQDef(MsgBox_MTsmokeMS, 2, &smokeMS_MEAS);          // ��Ϣ���ж���,��������ͨѶ�߳���ģ���߳�
osMessageQId  MsgBox_DPsmokeMS;
osMessageQDef(MsgBox_DPsmokeMS, 2, &smokeMS_MEAS);          // ��Ϣ���ж��壬����ģ���߳�����ʾģ���߳�

void smokeMS_DIOinit(void){

	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 
}

void smokeMS_AIOinit(void){

	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);	  //ʹ��ADC1ͨ��ʱ��
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M


	//PA0  ��Ϊģ��ͨ����������  ADC12_IN8                       

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	ADC_DeInit(ADC1);  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת���������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

  
	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1������ת����������
}

void smokeMS_Init(void){

	smokeMS_DIOinit();
	smokeMS_AIOinit();
}

//���ADCֵ
//ch:ͨ��ֵ 0~3
uint16_t smokeGet_Adc(uint8_t ch)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1������ת����������	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
}

uint16_t smokeGet_Adc_Average(uint8_t ch,uint8_t times)
{
	u32 temp_val=0;
	uint8_t t;
	
	for(t=0;t<times;t++)
	{
		temp_val += smokeGet_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	

//�۳�����LEDʱ��Ϊ����10MS���ߵ�ƽΪ0.32uS��
void smokeMS_led_Thread(const void *argument)
{
	unsigned char i;
	
	while (1)
	{
		int temp_buff = 0;
		for (i = 0; i < 10; ++i)
			{
			
			GPIO_SetBits(GPIOB,GPIO_Pin_8);			  
			delay_us(280);		
			temp_buff	+= (uint8_t)(smokeGet_Adc(0) / 41);//���ݲɼ�
			delay_us(40);	  
			GPIO_ResetBits(GPIOB,GPIO_Pin_8); 		   
			delay_us(9680);
			
			}
		
		temp = temp_buff/10;	
		delay_ms(450);	
					
		}
}

void smokeMS_Thread(const void *argument){
	
	osEvent  evt;
    osStatus status;
	
	const bool UPLOAD_MODE = false;	//1�����ݱ仯ʱ���ϴ� 0�����ڶ�ʱ�ϴ�
	
	const uint8_t upldPeriod = 5;	//�����ϴ�����������UPLOAD_MODE = false ʱ��Ч��
	
	uint8_t UPLDcnt = 0;
	bool UPLD_EN = false;
	
	const uint8_t dpSize = 30;
	const uint8_t dpPeriod = 20;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	smokeMS_MEAS	sensorData;
	static smokeMS_MEAS Data_temp = {1};
	static smokeMS_MEAS Data_tempDP = {1};
	
	smokeMS_MEAS *mptr = NULL;
	smokeMS_MEAS *rptr = NULL;
	
	for(;;){
		
		evt = osMessageGet(MsgBox_MTsmokeMS, 100);
		if (evt.status == osEventMessage) {		//�ȴ���Ϣָ��
			
			rptr = evt.value.p;
			/*�Զ��屾���߳̽������ݴ���������������������������*/
			

			do{status = osPoolFree(smokeMS_pool, rptr);}while(status != osOK);	//�ڴ��ͷ�
			rptr = NULL;
		}
		
		sensorData.anaDAT	= temp;		//���ݲɼ�
		//sensorData.VAL		= GAS_DATA;
		
		if(!UPLOAD_MODE){	//ѡ���ϴ�����ģʽ
		
			if(UPLDcnt < upldPeriod)UPLDcnt ++;
			else{
			
				UPLDcnt = 0;
				UPLD_EN = true;
			}
		}else{
			
			if(Data_temp.anaDAT != sensorData.anaDAT || Data_temp.VAL != sensorData.VAL){	//�������ͣ����ݸ���ʱ�Ŵ�����
			
				Data_temp.anaDAT = sensorData.anaDAT;
				Data_temp.VAL 	 = sensorData.VAL;
				UPLD_EN = true;
			}
		}
		
		if(UPLD_EN){
			
			UPLD_EN = false;
			
			do{mptr = (smokeMS_MEAS *)osPoolCAlloc(smokeMS_pool);}while(mptr == NULL);
			mptr->VAL = sensorData.VAL;
			mptr->anaDAT = sensorData.anaDAT;
			osMessagePut(MsgBox_smokeMS, (uint32_t)mptr, 100);
			osDelay(500);
		}
		
		if(Data_tempDP.anaDAT != sensorData.anaDAT || Data_tempDP.VAL != sensorData.VAL){	//�������ͣ����ݸ���ʱ�Ŵ�����
		
			Data_tempDP.anaDAT = sensorData.anaDAT;
			Data_tempDP.VAL    = sensorData.VAL;
			
			do{mptr = (smokeMS_MEAS *)osPoolCAlloc(smokeMS_pool);}while(mptr == NULL);
			mptr->anaDAT = sensorData.anaDAT;
			mptr->VAL = sensorData.VAL;
			osMessagePut(MsgBox_DPsmokeMS, (uint32_t)mptr, 100);
			osDelay(10);
		}
		
		if(Pcnt < dpPeriod){osDelay(10);Pcnt ++;}
		else{
		
			Pcnt = 0;
			memset(disp,0,dpSize * sizeof(char));
			sprintf(disp,"\n\rvalAnalog : %d%%,valDigital : %d\n\r", sensorData.anaDAT,sensorData.VAL);			
			Driver_USART1.Send(disp,strlen(disp));	
			osDelay(20);
		}
		
		osDelay(10);
	}
}

void smokeMSThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		smokeMS_pool   = osPoolCreate(osPool(smokeMS_pool));	//�����ڴ��
		MsgBox_smokeMS 	= osMessageCreate(osMessageQ(MsgBox_smokeMS), NULL);   //������Ϣ����
		MsgBox_MTsmokeMS = osMessageCreate(osMessageQ(MsgBox_MTsmokeMS), NULL);//������Ϣ����
		MsgBox_DPsmokeMS = osMessageCreate(osMessageQ(MsgBox_DPsmokeMS), NULL);//������Ϣ����
		
		memInit_flg = true;
	}

	smokeMS_Init();
	tid_smokeMS_Thread = osThreadCreate(osThread(smokeMS_Thread),NULL);
	tid_smokeMS_led_Thread	= osThreadCreate(osThread(smokeMS_led_Thread),NULL);
}
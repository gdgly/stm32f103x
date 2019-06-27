/*---------------------------------------------------------------------------
 *
 * Copyright (C),2014-2019, guoshun Tech. Co., Ltd.
 *
 * @Project:    智能实训台项目
 * @Version:    V 0.2 
 * @Module:     intrrupt
 * @Author:     RanHongLiang
 * @Date:       2019-06-26 13:12:49
 * @Description: 
 *————中断服务函数，主要用于红外编码学习
 *---------------------------------------------------------------------------*/


#include "intrrupt.h"
//文件内包括用户自定义中断函数管理，目前仅有红外转发进程中红外编码学习的外部中断处理函数；
extern ARM_DRIVER_USART Driver_USART1;

void EXTI9_5_IRQHandler(void) {
	
	switch(Moudle_GTA.Extension_ID){
	
		case MID_EXEC_DEVIFR:{
			
				const uint8_t IFR_PER  = 2;		//电平采样分辨率 2us
				const uint8_t BUF_WARN = 200;	//缓存溢出警告值
				
				const uint16_t SIGLEN_MAX = 50000 / IFR_PER;  //一个电平信号维持最长时间限制
				const uint16_t SIGLEN_MIN = 6000 / IFR_PER;  //一个电平信号维持最短时间限制
				
				static uint8_t Tab_Hp,Tab_Lp;
				static uint16_t HT_Tab[Tab_size] = {0};
				static uint16_t LT_Tab[Tab_size] = {0};
				uint16_t time = 0;
				
				Tab_Hp = Tab_Lp = 0;
				
				if(measure_en && EXTI_GetITStatus(EXTI_Line6) == SET){
					
					Tab_Hp = Tab_Lp = 0;
					memset(HT_Tab, 0, Tab_size * sizeof(uint16_t));
					memset(LT_Tab, 0, Tab_size * sizeof(uint16_t));
					while(1){
					
						if(!PCin(6)){
						
							time = LW_ReceiveTime();
							
							if(time > SIGLEN_MIN && Tab_Lp > BUF_WARN){  //缓存不够，周期截波跳出
							
								//Driver_USART1.Send(&Tab_Lp,1);   //单周期高电平信号总长输出测试
								memcpy((uint16_t *)LTtab,LT_Tab,Tab_Lp * 2);	//数据类型为 u16,而memcpy以字节为单位，倍乘2
								memcpy((uint16_t *)HTtab,HT_Tab,Tab_Hp * 2);
								tabLp = Tab_Lp;
								tabHp = Tab_Hp;
								break;
							}else{
								
								LT_Tab[Tab_Lp ++] = time;
							}
						}
						if(PCin(6)){
						
							time = HW_ReceiveTime();
								
							if((time > SIGLEN_MAX) || (time > SIGLEN_MIN && Tab_Hp > BUF_WARN)){
								
								//Driver_USART1.Send(&Tab_Hp,1);	//单周期低电平信号总长输出测试
								memcpy((uint16_t *)LTtab,LT_Tab,Tab_Lp * 2);	//数据类型为 u16,而memcpy以字节为单位，倍乘2
								memcpy((uint16_t *)HTtab,HT_Tab,Tab_Hp * 2);
								tabLp = Tab_Lp;
								tabHp = Tab_Hp;
								break;
							}else{

								HT_Tab[Tab_Hp ++] = time;
							}
						}
					}
					measure_en = false;
				}
			}break;

		default:break;
	}
	
	EXTI_ClearITPendingBit(EXTI_Line6);
	EXTI_ClearFlag(EXTI_Line6);
}


#include "delay.h"//文件内包括延时函数；
/*---------------------------------------------------------------------------
 *
 * Copyright (C),2014-2019, guoshun Tech. Co., Ltd.
 *
 * @Project:    智能实训台项目
 * @Version:    V 0.2 
 * @Module:     delay
 * @Author:     RanHongLiang
 * @Date:       2019-06-26 13:14:13
 * @Description: 
 *————bsp级别延时函数，声明和初始化，寄存器版本
 *---------------------------------------------------------------------------*/
/*
*********************************************************************************************************
*                                             寄存器
*********************************************************************************************************
*/
#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)

/*
*********************************************************************************************************
*	函 数 名: bsp_InitDWT
*	功能说明: 初始化DWT. 该函数被 bsp_Init() 调用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_delayinit(void)
{
	DEM_CR         |= (unsigned int)DEM_CR_TRCENA;   /* Enable Cortex-M4's DWT CYCCNT reg.  */
	DWT_CYCCNT      = (unsigned int)0u;
	DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}

void delay_us(uint32_t _ulDelayTime)
{
   uint32_t tCnt, tDelayCnt;
	uint32_t tStart;
		
	tStart = DWT_CYCCNT;                                     /* 刚进入时的计数器值 */
	tCnt = 0;
	tDelayCnt = _ulDelayTime * (SystemCoreClock / 1000000);	 /* 需要的节拍数 */ 		      

	while(tCnt < tDelayCnt)
	{
		tCnt = DWT_CYCCNT - tStart; /* 求减过程中，如果发生第一次32位计数器重新计数，依然可以正确计算 */	
	}
}

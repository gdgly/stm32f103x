#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"
#include "stm32f10x_dma.h"
 	    					    
#define   UART1_DMA_TX_CH   DMA1_Channel4
#define   UART1_DMA_RX_CH   DMA1_Channel5

#define   RS485_COMM_DMA_TX_CH   DMA1_Channel7
#define   RS485_COMM_DMA_RX_CH   DMA1_Channel6

 
 

 
extern void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr, uint32_t  dir,uint32_t  tDMA_Mode );//����DMA1_CHx

extern void MYDMATX_Enable(DMA_Channel_TypeDef*DMA_CHx,u16 cndtr);//ʹ��DMA1_CHx
		   
#endif





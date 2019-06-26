#include "lcd.h"
#include "stdlib.h"
#include "font.h" 
#include "adc.h"	 
#include "delay.h"	   
#include "spi.h"	 
 			
extern u16 dis;
extern u16 discRanging;
//LCD�Ļ�����ɫ�ͱ���ɫ	   
u16 POINT_COLOR=0x0000;	//������ɫ
u16 BACK_COLOR=GREEN;  //����ɫ 

//����LCD��Ҫ����
_lcd_dev lcddev;
	
		   
//д�Ĵ�������
//regval:�Ĵ���ֵ
void LCD_WR_REG(u16 regval)
{ 
	SPILCD_CS_RESET;  //LCD_CS=0
    SPILCD_RS_RESET;
	SPI_WriteByte(SPI2,regval&0x00FF);
	SPILCD_CS_SET;  //LCD_CS=1	   		 
}
//дLCD����
//data:Ҫд���ֵ
void LCD_WR_DATA(u16 data)
{
 	SPILCD_CS_RESET;  //LCD_CS=0
	SPILCD_RS_SET;	
	SPI_WriteByte(SPI2,data>>8);
	SPI_WriteByte(SPI2,data);
	SPILCD_CS_SET;  //LCD_CS=1		
}
void LCD_WR_DATA8(u8 da)   //д8λ����
{
	SPILCD_CS_RESET;  //LCD_CS=0
	SPILCD_RS_SET;				    	   
	SPI_WriteByte(SPI2,da);	
	SPILCD_CS_SET;  //LCD_CS=1   			 
}					   
//д�Ĵ���
//LCD_Reg:�Ĵ�����ַ
//LCD_RegValue:Ҫд�������
void LCD_WR_REG_DATA(u8 LCD_Reg, u16 LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}
//��ʼдGRAM
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);  
}	 
//��mdk -O1ʱ���Ż�ʱ��Ҫ����
//��ʱi
void opt_delay(u8 i)
{
	while(i--);
}  		 
//LCD������ʾ
void LCD_DisplayOn(void)
{					   

}	 
//LCD�ر���ʾ
void LCD_DisplayOff(void)
{	   

}   

//���ù��λ��
//Xpos:������
//Ypos:������
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
    LCD_WR_REG(lcddev.setxcmd); 
	LCD_WR_DATA8(Xpos>>8); 
	LCD_WR_DATA8(Xpos&0XFF);	 
	LCD_WR_REG(lcddev.setycmd); 
	LCD_WR_DATA8(Ypos>>8); 
	LCD_WR_DATA8(Ypos&0XFF);
} 	  

//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);		//���ù��λ�� 
	LCD_WriteRAM_Prepare();	//��ʼд��GRAM
	LCD_WR_DATA(POINT_COLOR); 
} 
//��ʼ��lcd
void LCD_Init(void)
{ 	 
	SPILCD_RST_RESET ;	
	delay_ms(20); 
    SPILCD_RST_SET ;		
	delay_ms(20);

	lcddev.width=320;  //240;
	lcddev.height=240;  //320;
	lcddev.wramcmd=0X2C;
	lcddev.setxcmd=0X2A;
	lcddev.setycmd=0X2B; 	

		LCD_WR_REG(0xCB);  
        LCD_WR_DATA8(0x39); 
        LCD_WR_DATA8(0x2C); 
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0x34); 
        LCD_WR_DATA8(0x02); 

        LCD_WR_REG(0xCF);  
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0XC1); 
        LCD_WR_DATA8(0X30); 
 
        LCD_WR_REG(0xE8);  
        LCD_WR_DATA8(0x85); 
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0x78); 
 
        LCD_WR_REG(0xEA);  
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0x00); 
 
        LCD_WR_REG(0xED);  
        LCD_WR_DATA8(0x64); 
        LCD_WR_DATA8(0x03); 
        LCD_WR_DATA8(0X12); 
        LCD_WR_DATA8(0X81); 

        LCD_WR_REG(0xF7);  
        LCD_WR_DATA8(0x20); 
  
        LCD_WR_REG(0xC0);    //Power control 
        LCD_WR_DATA8(0x23);   //VRH[5:0] 
 
        LCD_WR_REG(0xC1);    //Power control 
        LCD_WR_DATA8(0x10);   //SAP[2:0];BT[3:0] 
 
        LCD_WR_REG(0xC5);    //VCM control 
        LCD_WR_DATA8(0x3e); //�Աȶȵ���
        LCD_WR_DATA8(0x28); 
 
        LCD_WR_REG(0xC7);    //VCM control2 
        LCD_WR_DATA8(0x86);  //--

	LCD_WR_REG(0x36);    // Memory Access Control 
	LCD_WR_DATA8(0xE8); //C8	   //48 68����//28 E8 ����
	
#if 1	
	LCD_WR_REG(0x2B);     //set the page address ����Ļ����
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0xEF);
	LCD_WR_REG(0x2A);    //set the column address
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x01);
	LCD_WR_DATA8(0x3F);
#else
	LCD_WR_REG(0x2A);     //set the page address ����Ļ����
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0xEF);
	LCD_WR_REG(0x2B);    //set the column address
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x01);
	LCD_WR_DATA8(0x3F);    
#endif

        LCD_WR_REG(0x3A);    
        LCD_WR_DATA8(0x55); 

        LCD_WR_REG(0xB1);    
        LCD_WR_DATA8(0x00);  
        LCD_WR_DATA8(0x18); 
 
        LCD_WR_REG(0xB6);    // Display Function Control 
        LCD_WR_DATA8(0x08); 
        LCD_WR_DATA8(0x82);
        LCD_WR_DATA8(0x27);  
 
        LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
        LCD_WR_DATA8(0x00); 
 
        LCD_WR_REG(0x26);    //Gamma curve selected 
        LCD_WR_DATA8(0x01); 
 
        LCD_WR_REG(0xE0);    //Set Gamma 
        LCD_WR_DATA8(0x0F); 
        LCD_WR_DATA8(0x31); 
        LCD_WR_DATA8(0x2B); 
        LCD_WR_DATA8(0x0C); 
        LCD_WR_DATA8(0x0E); 
        LCD_WR_DATA8(0x08); 
        LCD_WR_DATA8(0x4E); 
        LCD_WR_DATA8(0xF1); 
        LCD_WR_DATA8(0x37); 
        LCD_WR_DATA8(0x07); 
        LCD_WR_DATA8(0x10); 
        LCD_WR_DATA8(0x03); 
        LCD_WR_DATA8(0x0E); 
        LCD_WR_DATA8(0x09); 
        LCD_WR_DATA8(0x00); 

        LCD_WR_REG(0XE1);    //Set Gamma 
        LCD_WR_DATA8(0x00); 
        LCD_WR_DATA8(0x0E); 
        LCD_WR_DATA8(0x14); 
        LCD_WR_DATA8(0x03); 
        LCD_WR_DATA8(0x11); 
        LCD_WR_DATA8(0x07); 
        LCD_WR_DATA8(0x31); 
        LCD_WR_DATA8(0xC1); 
        LCD_WR_DATA8(0x48); 
        LCD_WR_DATA8(0x08); 
        LCD_WR_DATA8(0x0F); 
        LCD_WR_DATA8(0x0C); 
        LCD_WR_DATA8(0x31); 
        LCD_WR_DATA8(0x36); 
        LCD_WR_DATA8(0x0F); 
 
        LCD_WR_REG(0x11);    //Exit Sleep 
        delay_ms(120);
				
	LCD_WR_REG(0x29);    //Display on 
	LCD_WR_REG(0x2c); 
	LCD_Clear(GREEN);

}  
//��������
//color:Ҫ���������ɫ
void LCD_Clear(u16 color)
{
	u32 index=0;      
	u32 totalpoint=lcddev.width;
	totalpoint*=lcddev.height; 	//�õ��ܵ���
	LCD_SetCursor(0x00,0x0000);	//���ù��λ�� 
	LCD_WriteRAM_Prepare();     //��ʼд��GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD_WR_DATA(color);
	}

}  
//��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{          
	u16 i,j;
	u16 xlen=0;
	xlen=ex-sx+1;	   
	for(i=sy;i<=ey;i++)
	{									   
	 	LCD_SetCursor(sx,i);      				//���ù��λ�� 
		LCD_WriteRAM_Prepare();     			//��ʼд��GRAM	  
		for(j=0;j<xlen;j++)LCD_WR_DATA(color);	//���ù��λ�� 	    
	}
}  
//��ָ�����������ָ����ɫ��			 
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{  
	u16 height,width;
	u16 i,j;
	width=ex-sx+1; 		//�õ����Ŀ���
	height=ey-sy+1;		//�߶�
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//���ù��λ�� 
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM
		for(j=0;j<width;j++)LCD->LCD_RAM=color[i*height+j];//д������ 
	}	  
}  
//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		LCD_DrawPoint(uRow,uCol);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    
//������	  
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b);             //5
 		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-a,y0+b);             //1       
 		LCD_DrawPoint(x0-b,y0+a);             
		LCD_DrawPoint(x0-a,y0-b);             //2             
  		LCD_DrawPoint(x0-b,y0-a);             //7     	         
		a++;
		//ʹ��Bresenham�㷨��Բ     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 	
//��ָ��λ����ʾһ������(16*16��С)
void showhanzi16(unsigned int x,unsigned int y,unsigned char index)	
{  
	unsigned char i,j,k;
	const unsigned char *temp=hanzi16[index];    

	for(j=0;j<16;j++)
	{
		LCD_SetCursor(x,y+j);
		LCD_WriteRAM_Prepare();	//��ʼд��GRAM
		for(k=0;k<2;k++)
		{
			for(i=0;i<8;i++)
			{ 		     
			 	if((*temp&(1<<i))!=0)
				{
					LCD_WR_DATA(POINT_COLOR);
				} 
				else
				{
					LCD_WR_DATA(BACK_COLOR);
				}   
			}
			temp++;
		}
	 }
}	

void showhanzi24(unsigned int x,unsigned int y,unsigned char index)	
{  
	unsigned char i,j,k;
	const unsigned char *temp=hanzi24[index];    

	for(j=0;j<24;j++)
	{
		LCD_SetCursor(x,y+j);
		LCD_WriteRAM_Prepare();	//��ʼд��GRAM
		for(k=0;k<3;k++)
		{
			for(i=0;i<8;i++)
			{ 		     
			 	if((*temp&(1<<i))!=0)
				{
					LCD_WR_DATA(POINT_COLOR);
				} 
				else
				{
					LCD_WR_DATA(BACK_COLOR);
				}   
			}
			temp++;
		}
	 }
}	


//��ָ��λ����ʾһ������(32*32��С)
void showhanzi32(unsigned int x,unsigned int y,const unsigned char *pbuf)	
{  
	unsigned char i,j,k;
	const unsigned char *temp=pbuf;    

	for(j=0;j<32;j++)
	{
		LCD_SetCursor(x,y+j);
		LCD_WriteRAM_Prepare();	//��ʼд��GRAM
		for(k=0;k<4;k++)
		{
			for(i=0;i<8;i++)
			{ 		     
			 	if((*temp&(1<<i))!=0)
				{
					LCD_WR_DATA(POINT_COLOR);
				} 
				else
				{
					LCD_WR_DATA(BACK_COLOR);
				}   
			}
			temp++;
		}
	 }
}	


//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  							  
    	u8 temp,t1,t,byteNum;
	u16 y0=y;
	u16 colortemp=POINT_COLOR;      			     
	//���ô���		   

	if(size==56)
	{
		num=num-'0';//�õ�ƫ�ƺ��ֵ
		byteNum =196;
	}
	else if(size==32)
	{
		num=num-' ';//�õ�ƫ�ƺ��ֵ
		byteNum =64;
	}
	else	
	{
		num=num-' ';//�õ�ƫ�ƺ��ֵ
		byteNum =size;
	}
		
	
	if(!mode) //�ǵ��ӷ�ʽ
	{
	    for(t=0;t<byteNum;t++)
	    {   
			if(size==12)temp=asc2_1206[num][t];  //����1206����
			else if(size==16) temp=asc2_1608[num][t];		 //����1608���� 	                          
			else if(size==32) temp=asc2_3216[num][t];		 //����1608���� 
			//else if(size==56) temp=asc2_5628[num][t];		 //����1608���� 				
			else break;
	        for(t1=0;t1<8;t1++)
			{			    
		        if(temp&0x80)POINT_COLOR=colortemp;
				else POINT_COLOR=BACK_COLOR;
				LCD_DrawPoint(x,y);	
				temp<<=1;
				y++;
				if(y>=lcddev.height){POINT_COLOR=colortemp;return;}//��������
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
					break;
				}
			}  	 
	    }    
	}else//���ӷ�ʽ
	{
	    for(t=0;t<byteNum;t++)
	    {   
			if(size==12)temp=asc2_1206[num][t];  //����1206����
			else if(size==16) temp=asc2_1608[num][t];		 //����1608���� 	                          
			else if(size==32) temp=asc2_3216[num][t];		 //����1608���� 
			//else if(size==56) temp=asc2_5628[num][t];		 //����1608���� 				
			else break;
	        for(t1=0;t1<8;t1++)
			{			    
		        if(temp&0x80)LCD_DrawPoint(x,y); 
				temp<<=1;
				y++;
				if(y>=lcddev.height){POINT_COLOR=colortemp;return;}//��������
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
					break;
				}
			}  	 
	    }     
	}
	POINT_COLOR=colortemp;	    	   	 	  
}   
//m^n����
//����ֵ:m^n�η�.
u32 LCD_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ 
//num:��ֵ(0~4294967295);	 
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
} 
//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);	 
//len:����(��Ҫ��ʾ��λ��)
//size:�����С
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode)
{  
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);  
				else LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);  
 				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01); 
	}
} 
//��ʾ�ַ���
//x,y:�������
//width,height:�����С  
//size:�����С
//*p:�ַ�����ʼ��ַ		  
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{         
	u8 x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//�˳�
        LCD_ShowChar(x,y,*p,size,1);
        x+=size/2;
        p++;
    }  
}


extern u16 curSpeed;
extern u8 CarTurn;
extern u16 discRanging;

void LcdShowCarMsg(void) 
{
	u16 star_x;
	u16 adcx;
	float temp;

	BACK_COLOR=GREEN;
	POINT_COLOR=BRED;   

	star_x = (lcddev.width-32*4)/2;		
	showhanzi32(star_x,25,hanzi32[Z32_ZHI]);    
	showhanzi32(star_x+32,25,hanzi32[Z32_NENG]);    
	showhanzi32(star_x+2*32,25,hanzi32[Z32_XIAO]);    
	showhanzi32(star_x+3*32,25,hanzi32[Z32_CHE]); 
	
	POINT_COLOR=BLUE;   
	star_x = (lcddev.width-32*6)/2;		
	showhanzi32(star_x,75,hanzi32[Z32_CHE]);    
	showhanzi32(star_x+32,75,hanzi32[Z32_SU]);    
	showhanzi32(star_x+2*32,75,hanzi32[Z32_MAOHAO]);    
	POINT_COLOR=RED;   
	LCD_ShowNum(star_x+2*32+26,75,curSpeed,3,32);
//	LCD_ShowNum(star_x+2*32+26,75,discRanging,3,32);
	LCD_ShowString(star_x+2*32+90,75,32*4,32,32,"mm/s");

	POINT_COLOR=BLUE;   
	star_x = (lcddev.width-32*6)/2;		
	showhanzi32(star_x,125,hanzi32[Z32_ZHUAN]);    
	showhanzi32(star_x+32,125,hanzi32[Z32_XIANG]);    
	showhanzi32(star_x+2*32,125,hanzi32[Z32_MAOHAO]);    

	POINT_COLOR=RED;   
	switch(CarTurn)
	{
		case 1:	// ��ת
			showhanzi32(star_x+3*32,125,hanzi32[Z32_ZUO]);    
			showhanzi32(star_x+4*32,125,hanzi32[Z32_ZHUAN]);    			
		break;
		
		case 2:	// ��ת
			showhanzi32(star_x+3*32,125,hanzi32[Z32_YOU]);    
			showhanzi32(star_x+4*32,125,hanzi32[Z32_ZHUAN]);    			
		break;
		
		default:	// ֱ��
			showhanzi32(star_x+3*32,125,hanzi32[Z32_ZI]);    
			showhanzi32(star_x+4*32,125,hanzi32[Z32_XING]);    
		break;
		
	}

	POINT_COLOR=BLUE;   
	star_x = (lcddev.width-32*6)/2;		
	showhanzi32(star_x,175,hanzi32[Z32_DIAN]);    
	showhanzi32(star_x+32,175,hanzi32[Z32_YA]);    
	showhanzi32(star_x+2*32,175,hanzi32[Z32_MAOHAO]);    

	POINT_COLOR=RED;   
	temp = (float) (Get_Adc_Average(ADC_Channel_5,10)) *7.95/4095;
	adcx = temp;
	LCD_ShowxNum(star_x+3*32,175,adcx,1,32,0);
	LCD_ShowChar(star_x+3*32+16,175,'.',32,0);
	temp = (temp - adcx)*1000;
	LCD_ShowxNum(star_x+4*32,175,temp,3,32,0X80);				
	LCD_ShowChar(star_x+6*32,175,'V',32,0);			
}



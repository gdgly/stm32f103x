#include "lightMS.h"//����ǿ�ȼ���������̺�����


//IO��ʼ��
void tsl2561IO_Init(void)
{
 
 	GPIOB->DDR |= GPIO_PIN_5;//���ģʽ
	GPIOB->CR1 &=~ GPIO_PIN_5;//��©��� 
	
	GPIOB->DDR |= GPIO_PIN_4;//���ģʽ
	GPIOB->CR1 |= GPIO_PIN_4;//������� 	
}

void TSL2561_Init(void)
{
	tsl2561IO_Init();
	TSLSDA_OUT();
	TSLIIC_SCL=1;
	TSLIIC_SDA=1;
	TSL2561_Write(CONTROL,0x03);
	delay_ms(100);
	TSL2561_Write(TIMING,0x02);
}

void tsl2561_start(void)
{
	TSLSDA_OUT();     //sda
	TSLIIC_SDA=1;	  	  
	TSLIIC_SCL=1;
	delay_us(4);
 	TSLIIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	TSLIIC_SCL=0;//
}

void stop(void)
{
	TSLSDA_OUT();//sda
	TSLIIC_SCL=0;
	TSLIIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	TSLIIC_SCL=1; 
	TSLIIC_SDA=1;//
	delay_us(4);							   	
}

void respons(void)
{
	TSLIIC_SCL=0;
	TSLSDA_OUT();
	TSLIIC_SDA=0;
	delay_us(2);
	TSLIIC_SCL=1;
	delay_us(2);
	TSLIIC_SCL=0;
}

void write_byte(u8 value)
{
    uint8_t t;   
	TSLSDA_OUT(); 	    
    TSLIIC_SCL=0;
    for(t=0;t<8;t++)
    {              
        //TSLIIC_SDA=(txd&0x80)>>7;
		if((value&0x80)>>7)
			TSLIIC_SDA=1;
		else
			TSLIIC_SDA=0;
		value<<=1; 	  
		delay_us(2);   
		TSLIIC_SCL=1;
		delay_us(2); 
		TSLIIC_SCL=0;	
		delay_us(2);
    }	 
}

u8 read_byte(void)
{
	unsigned char i,receive=0;
	TSLSDA_IN();//SDA
    for(i=0;i<8;i++ )
	{
        TSLIIC_SCL=0; 
        delay_us(2);
		TSLIIC_SCL=1;
        receive<<=1;
        if(TSLREAD_SDA)receive++;   
		delay_us(1); 
    }					 

	TSLSDA_OUT();
	TSLIIC_SDA=1;//release DATA-line
	return receive;
}


void TSL2561_Write(u8 command,u8 data)
{
	tsl2561_start();
	write_byte(SLAVE_ADDR_WR);
	respons();
	write_byte(command);
	respons();
	write_byte(data);
	respons();
	stop();
}


u8 TSL2561_Read(u8 command)
{
	uint8 data;
	tsl2561_start();
	write_byte(SLAVE_ADDR_WR);
	respons();
	write_byte(command);
	respons();
	
	tsl2561_start();
	write_byte(SLAVE_ADDR_RD);
	respons();
	data=read_byte();
	stop();
	return data;
}

u32 Read_Light(void)
{
	uint16 Channel0,Channel1;
	uint8 Data0_L,Data0_H,Data1_L,Data1_H;
	
	Data0_L=TSL2561_Read(DATA0LOW);
	Data0_H=TSL2561_Read(DATA0HIGH);
	Channel0=(256*Data0_H + Data0_L);
	
	Data1_L=TSL2561_Read(DATA1LOW);
	Data1_H=TSL2561_Read(DATA1HIGH);
	Channel1=(256*Data1_H + Data1_L);
	
	return calculateLux(Channel0,Channel1);
	
	//return 0;
}

u32 calculateLux(u16 ch0, u16 ch1)
{
		uint32_t chScale;
		uint32_t channel1;
		uint32_t channel0;
		uint32_t temp;
		uint32_t ratio1 = 0;
		uint32_t ratio	;
		uint32_t lux_temp;
		uint16_t b, m;
		chScale=(1 <<TSL2561_LUX_CHSCALE);           //����ʱ��Ĵ���Ϊ0x02��
		chScale = chScale << 4;                      //��������Ϊ1�ģ�����Ϊ16����д��һ��
		// scale the channel values
		channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
		channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;
		// find the ratio of the channel values (Channel1/Channel0)
		if (channel0 != 0)
		ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;
		ratio = (ratio1 + 1) >> 1;	  									 // round the ratio value
		if ((ratio > 0) && (ratio <= TSL2561_LUX_K1T))
			{
				b=TSL2561_LUX_B1T;
				m=TSL2561_LUX_M1T;
			}
		else if (ratio <= TSL2561_LUX_K2T)
			{
				b=TSL2561_LUX_B2T;
				m=TSL2561_LUX_M2T;
			}
		else if (ratio <= TSL2561_LUX_K3T)
			{
				b=TSL2561_LUX_B3T;
				m=TSL2561_LUX_M3T;
			}
		else if (ratio <= TSL2561_LUX_K4T)
			{
				b=TSL2561_LUX_B4T;
				m=TSL2561_LUX_M4T;
			}
		else if (ratio <= TSL2561_LUX_K5T)
			{
				b=TSL2561_LUX_B5T;
				m=TSL2561_LUX_M5T;
			}
		else if (ratio <= TSL2561_LUX_K6T)
			{
				b=TSL2561_LUX_B6T;
				m=TSL2561_LUX_M6T;
			}
		else if (ratio <= TSL2561_LUX_K7T)
			{
				b=TSL2561_LUX_B7T;
				m=TSL2561_LUX_M7T;
			}
		else if (ratio > TSL2561_LUX_K8T)
			{
				b=TSL2561_LUX_B8T;
				m=TSL2561_LUX_M8T;
			}
		temp = ((channel0 * b) - (channel1 * m));
		if (temp < 1)  temp = 0;							// do not allow negative lux value
		temp += (1 << (TSL2561_LUX_LUXSCALE-1));			// round lsb (2^(LUX_SCALE-1))
		lux_temp = temp >> TSL2561_LUX_LUXSCALE;			// strip off fractional portion
		return lux_temp;		  							// Signal I2C had no errors
}

void lightMS_Init(void){

	TSL2561_Init();

}

void lightMS_Thread( u8 *argument){
	u32 buff;


	lightMS_Init();
	buff = Read_Light();
	memcpy(argument,&buff,4);
	
}


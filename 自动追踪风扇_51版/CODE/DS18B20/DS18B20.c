#include <DS18B20.H>

//��������ʱ����
void Delay_OneWire(unsigned int t)  
{
	while(t--) ;
}


//ͨ����������DS18B20дһ���ֽ�
void Write_DS18B20(unsigned char dat)
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		DQ = 0;
		DQ = dat&0x01;
		Delay_OneWire(10);
		DQ = 1;
		dat >>= 1;
	}
	Delay_OneWire(5);
}

//��DS18B20��ȡһ���ֽ�
unsigned char Read_DS18B20(void)
{
	unsigned char i;
	unsigned char dat;
  
	for(i=0;i<8;i++)
	{
		DQ = 0;
		dat >>= 1;
		DQ = 1;
		if(DQ)
		{
			dat |= 0x80;
		}	    
		Delay_OneWire(4);
	}
	return dat;
}

//DS18B20�豸��ʼ��
void init_ds18b20(void)
{
  	bit initflag = 0;
  	
  	DQ = 1;
  	Delay_OneWire(8);
  	DQ = 0;
  	Delay_OneWire(80);
  	DQ = 1;
  	Delay_OneWire(14); 
    initflag = DQ;     
  	Delay_OneWire(20);
  
}

uchar readtemp()
{
	uchar temp,tph,tpl;
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0x44);
	while(!DQ);
	init_ds18b20();
	Write_DS18B20(0xcc);
	Write_DS18B20(0xbe);
	tpl = Read_DS18B20();
	tph = Read_DS18B20();
	temp = (tph<<4)|(tpl>>4);
	
	return temp;
}
#include "uart.h"
#include "LCD.H"  
#include "intrins.h"
#include "string.h"
#include "stdio.h"

unsigned int sec,Rec;

char buff[15];
static  uchar datas[10];

static uchar USART_RX_CNT = 0;
/* �����жϽ��յ����ݱ���Ļ����� */
static uchar USART_RX_BUF[64];
/* ���ڱ�ʶ���ڽ������ݰ��Ƿ���ɱ�־ */
static uchar From_Flag = 0;
/* ͨѶ��־ �����������ݺ���1 ���յ�Ӧ������� */
uchar RS485Busy = 0;
/* ���ջ����� */
uchar RS485_RX_BUF[64];  	//���ջ���,���64���ֽ�.
/* ���ڱ���������õ����� */
uint ReadDateVal = 0;

const unsigned char TabH[] = {  //CRC��λ�ֽ�ֵ��
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  
        0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
        0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,  
        0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,  
        0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
        0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
        0x80, 0x41, 0x00, 0xC1, 0x81, 0x40  
    } ;  
const unsigned char TabL[] = {  //CRC��λ�ֽ�ֵ��
        0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,  
        0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,  
        0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,  
        0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,  
        0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,  
        0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,  
        0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,  
        0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,  
        0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,  
        0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,  
        0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,  
        0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,  
        0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,  
        0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,  
        0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,  
        0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,  
        0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,  
        0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,  
        0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,  
        0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,  
        0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,  
        0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,  
        0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,  
        0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,  
        0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,  
        0x43, 0x83, 0x41, 0x81, 0x80, 0x40  
    } ;

/*************************************************************************************
 * ����˵��: CRC16У��
 * ��ڲ�����u8 *ptr,u8 len
 * ���ڲ�����u16
 * �������ܣ�������ڲ��������ֵ����crc16У��ֵ ������
**************************************************************************************/
unsigned int GetCRC16(unsigned char *pPtr,unsigned char ucLen)
{ 
    unsigned int  uiIndex;
    unsigned char ucCrch = 0xFF;  		//��CRC�ֽ�
    unsigned char ucCrcl = 0xFF;  		//��CRC�ֽ� 
    while (ucLen --)  			//����ָ������CRC
    {
        uiIndex = ucCrch ^ *pPtr++;
        ucCrch  = ucCrcl ^ TabH[uiIndex];
        ucCrcl  = TabL[uiIndex];
    }
    
    return ((ucCrch << 8) | ucCrcl);  
} 

/****************************************************************************************************
 * �������ƣ� void Send_Data(uchar *buf,uchar len)
 * ��ڲ�����uchar *buf uchar len
 * ����  ֵ����
 * ����˵�������ڷ�������
 * 			 buf:�������׵�ַ
 *			 len:���͵��ֽ���(Ϊ�˺ͱ�����Ľ���ƥ��,���ｨ�鲻Ҫ����64���ֽ�)
 ***************************************************************************************************/
void Send_Data(uchar *buf,uchar len)
{
	uchar t;
	delay(1);
	for(t=0;t<len;t++)		//ѭ����������
	{		
    SBUF=buf[t];	
  	while(!TI);
	  TI=0;		
	}	 
	delay(1);
	USART_RX_CNT=0;	  
}

/****************************************************************************************************
 * �������ƣ�uchar UartRead(uchar *buf, uchar len) 
 * ��ڲ�����uchar *buf uchar len
 * ����  ֵ��uchar
 * ����˵����������յ����ݳ��ȣ����ҽ����ݷŵ�*buf������
 ***************************************************************************************************/             
uchar UartRead(uchar *buf, uchar len)  
{
	uchar i;
	if(len > USART_RX_CNT)  		//ָ����ȡ���ȴ���ʵ�ʽ��յ������ݳ���ʱ
	{
		len = USART_RX_CNT; 		//��ȡ��������Ϊʵ�ʽ��յ������ݳ���
	}
	
	for(i = 0;i < len;i ++)  		//�������յ������ݵ�����ָ����
	{
		*buf = USART_RX_BUF[i];  	//�����ݸ��Ƶ�buf��
		buf  ++;
	}
	USART_RX_CNT=0;              	//���ռ���������
	return len;                   	//����ʵ�ʶ�ȡ����
}
/****************************************************************************************************
 * �������ƣ�void UartRxMonitor(uchar ms)
 * ��ڲ�����uchar ms
 * ����  ֵ����
 * ����˵�����ڶ�ʱ���е��ã����ڼ�����ݽ���
 ***************************************************************************************************/   
void UartRxMonitor(uchar ms) 					
{
	static uchar USART_RX_BKP = 0;  			//����USART2_RC_BKP��ʱ�洢ʫ�䳤����ʵ�ʳ��ȱȽ�
	static uchar idletmr      = 0;        		//������ʱ��
	
	if(USART_RX_CNT > 0)					//���ռ�����������ʱ��������߿���ʱ��
	{
		if(USART_RX_BKP != USART_RX_CNT) 	//���ռ������ı䣬���ս��յ�����ʱ��������м�ʱ
		{
			USART_RX_BKP = USART_RX_CNT;  	//��ֵ��������ʵ�ʳ��ȸ�USART2_RX_BKP
			idletmr      = 0;               //�����ʱ������
		}
		else                              	//���ռ�����δ�ı䣬�����߿���ʱ���ۼƿ���ʱ��
		{
											//�����һ֡�������֮ǰ�г���3.5���ֽ�ʱ���ͣ�٣������豸��ˢ�µ�ǰ����Ϣ���ٶ���һ���ֽ���һ���µ�����֡�Ŀ�ʼ
			if(idletmr < 3)                 //����ʱ��С��3msʱ�������ۼ�
			{
				idletmr += ms;
				if(idletmr >= 3)            //����ʱ��ﵽ3msʱ�����ж�Ϊ1֡�������
				{
					From_Flag = 1;			//����������־��֡������ϱ�־
				}
			}
		}
	}
	else
	{
		USART_RX_BKP = 0;USART_RX_BKP = USART_RX_CNT=0;return;
	}
}

/****************************************************************************************************
 * �������ƣ�void UartDriver(void)
 * ��ڲ�������
 * ����  ֵ����
 * ����˵�������������������������֡�Ľ��գ����ȹ��ܺ�����������ѭ���е���
 *           �����������������ڹ��ܵ��Ⱥ�����ֻ��Ҫ�õ�������ص����ݣ�
 *           �õ������ݸ������ѵ���Ҫʹ�ü��ɣ������ǻ���modbusЭ�飬����
 *           ��Ҫ����modbusЭ��ȥ��⣬������������modbus����
 ***************************************************************************************************/   
void UartDriver(void)
{
//	unsigned char i;
	
	unsigned int crc;
	unsigned char crch,crcl;
	static unsigned char len;
	static unsigned char buf[64];
	if (From_Flag==1)            									//֡������ɱ�־�������յ�һ֡������
	{
		From_Flag = 0;           								//֡������ɱ�־����
		len       = UartRead(buf,sizeof(buf));   				//�����յ������������������
		crc       = GetCRC16(buf,len-2);       					//����CRCУ��ֵ����ȥCRCУ��ֵ
		crch=crc  >> 8;    										//crc��λ
		crcl=crc  &  0xFF;										//crc��λ

		if((buf[len-2] == crch) && (buf[len-1] == crcl))  		//�ж�CRCУ���Ƿ���ȷ
		{
			if (buf[0] == 0x01)									//0x03 ������
			{
				if((buf[1] == 0x03) && (buf[2] <= 0x02))  		//�Ĵ�����ַ֧��0x0000~0x0005
				{
					/* ͨ���������֤�жϺ� �������ֱ�ӻ�ȡ���� ������ReadDateVal�� */
					ReadDateVal = buf[3] * 256 + buf[4];		//modbus������16λ��
					memset(buf, 0, sizeof(buf));  
         return;					
				}
			}
			/* д�����Ҫ����ֻ��ҪӦ�𼴿� */
			
			/* �ж�У������ȷ�� �����Ƕ�����д ������485æ��־����ʾ�յ�Ӧ���ͷ�485���ɽ�������������� */
//			RS485Busy = 0;										
		}
		else	memset(buf, 0, sizeof(buf)); return;   /* ���У��ֵ����ִ�д������� �����ô���1��ӡһ����ʾ��Ϣ Ҳ���Զ��干�������� */
			
	}
}

/****************************************************************************************************
 * �������ƣ�void RS485_RW_Opr(uchar ucAddr,uchar ucCmd,uint ucReg,uint uiDate)
 * ��ڲ�����uchar ucAddr,uchar ucCmd,uint ucReg,uint uiDate
 * 			 ucAddr���ӻ���ַ
 *			 ucCmd �������� 03->��	06->д
 *			 ucReg ���Ĵ�����ַ
 *			 uiDate��д�������Ƿ��͵����� ���������Ƕ�ȡ���ݸ���
 * ����  ֵ����
 * ����˵����485��д��������
 ***************************************************************************************************/   
void RS485_RW_Opr(uchar ucAddr,uchar ucCmd,uint ucReg,uint uiDate)
{
	unsigned int crc;
	unsigned char crcl;
	unsigned char crch;	
	unsigned char ucBuf[8];
	
	ucBuf[0] = ucAddr;				/* �ӻ���ַ */
	ucBuf[1] = ucCmd;				/* ���� 06 д 03 �� */
	ucBuf[2] = 0x00;//ucReg >> 8;			/* ������ַ��λ */
	ucBuf[3] = ucReg;				/* ��������ַ��λ */
	ucBuf[4] = 0x00;//uiDate >> 8;			/* ���ݸ�8λ */
	ucBuf[5] = uiDate;				/* ���ݵ�8λ */
	crc      = GetCRC16(ucBuf,6);   /* ����CRCУ��ֵ */
	crch     = crc >> 8;    		/* crc��λ */
	crcl     = crc &  0xFF;			/* crc��λ */
	ucBuf[6] = crch;				/* У���8λ */
	ucBuf[7] = crcl;				/* У���8λ */
	
	Send_Data(ucBuf,8);				/* �������� */
}


void LcdDisplay()
{
	uchar i;
	 datas[0]=ReadDateVal/1000;
   datas[1]=ReadDateVal%1000/100;
   datas[2]=ReadDateVal%1000%100/10;
   datas[3]=ReadDateVal%1000%100%10;
	
	
  for(i=0;i<5;i++)
	{
		write_1602com(0xc0);
	  write_1602dat('0'+datas[i]);
		if(i==3) write_1602dat('.');
	}
	
//	write_1602com(0xc0);
//  write_1602dat('0'+datas[0]);

//  write_1602com(0xc1);
//  write_1602dat('0'+datas[1]);

//  write_1602com(0xc2);
//  write_1602dat('0'+datas[2]);

//  write_1602com(0xc3);
//  write_1602dat('.');

//  write_1602com(0xc4);
//  write_1602dat('0'+datas[3]);

}

void main()
{
//	uchar i;
  Uart_init();
	lcd_init();
//	ReadDateVal=12;
	Send_String("welcome user");
 while(1)
 {
	 	if(sec>1000)  //1S�ɼ�һ������
	{
	  sec=0;
		RS485_RW_Opr(0x01,0x03,0x00,0x01);
	}

//   for(i=0;i<4;i++)
//   {
//  	SBUF=datas[i];
//  	while(!TI);
//	  TI=0;
//   }
//	 delay(8000);
//	 Send_String("welcome user");
	 UartDriver();
	 LcdDisplay();
 }

}

void timer0()interrupt 1
{
  TH0=0xfc;        //50ms
  TL0=0x18;
	UartRxMonitor(5);
	sec++;

}

/************************����1����********************/
void uart_isr() interrupt 4 using 2
{
  if(RI)
 {
  RI=0;
  Rec=SBUF; 
	 if(USART_RX_CNT<8)
	 {
	  	USART_RX_BUF[USART_RX_CNT] = Rec;			//��¼���յ���ֵ
	    USART_RX_CNT ++;								//������������1 
	 }
	 else {USART_RX_CNT=0; memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));return;} 
  Send_byte(Rec);	 
 }
  if(TI)  TI=0;
}




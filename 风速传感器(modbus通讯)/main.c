#include "uart.h"
#include "LCD.H"  
#include "intrins.h"
#include "string.h"
#include "stdio.h"

unsigned int sec,Rec;

char buff[15];
static  uchar datas[10];

static uchar USART_RX_CNT = 0;
/* 串口中断接收到数据保存的缓冲区 */
static uchar USART_RX_BUF[64];
/* 用于标识串口接收数据包是否完成标志 */
static uchar From_Flag = 0;
/* 通讯标志 主机发送数据后置1 接收到应答后清零 */
uchar RS485Busy = 0;
/* 接收缓存区 */
uchar RS485_RX_BUF[64];  	//接收缓冲,最大64个字节.
/* 用于保存读命令获得的数据 */
uint ReadDateVal = 0;

const unsigned char TabH[] = {  //CRC高位字节值表
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
const unsigned char TabL[] = {  //CRC低位字节值表
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
 * 函数说明: CRC16校验
 * 入口参数：u8 *ptr,u8 len
 * 出口参数：u16
 * 函数功能：根据入口参数数组的值计算crc16校验值 并返回
**************************************************************************************/
unsigned int GetCRC16(unsigned char *pPtr,unsigned char ucLen)
{ 
    unsigned int  uiIndex;
    unsigned char ucCrch = 0xFF;  		//高CRC字节
    unsigned char ucCrcl = 0xFF;  		//低CRC字节 
    while (ucLen --)  			//计算指定长度CRC
    {
        uiIndex = ucCrch ^ *pPtr++;
        ucCrch  = ucCrcl ^ TabH[uiIndex];
        ucCrcl  = TabL[uiIndex];
    }
    
    return ((ucCrch << 8) | ucCrcl);  
} 

/****************************************************************************************************
 * 函数名称： void Send_Data(uchar *buf,uchar len)
 * 入口参数：uchar *buf uchar len
 * 返回  值：无
 * 功能说明：串口发送数据
 * 			 buf:发送区首地址
 *			 len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
 ***************************************************************************************************/
void Send_Data(uchar *buf,uchar len)
{
	uchar t;
	delay(1);
	for(t=0;t<len;t++)		//循环发送数据
	{		
    SBUF=buf[t];	
  	while(!TI);
	  TI=0;		
	}	 
	delay(1);
	USART_RX_CNT=0;	  
}

/****************************************************************************************************
 * 函数名称：uchar UartRead(uchar *buf, uchar len) 
 * 入口参数：uchar *buf uchar len
 * 返回  值：uchar
 * 功能说明：计算接收的数据长度，并且将数据放到*buf数组中
 ***************************************************************************************************/             
uchar UartRead(uchar *buf, uchar len)  
{
	uchar i;
	if(len > USART_RX_CNT)  		//指定读取长度大于实际接收到的数据长度时
	{
		len = USART_RX_CNT; 		//读取长度设置为实际接收到的数据长度
	}
	
	for(i = 0;i < len;i ++)  		//拷贝接收到的数据到接收指针中
	{
		*buf = USART_RX_BUF[i];  	//将数据复制到buf中
		buf  ++;
	}
	USART_RX_CNT=0;              	//接收计数器清零
	return len;                   	//返回实际读取长度
}
/****************************************************************************************************
 * 函数名称：void UartRxMonitor(uchar ms)
 * 入口参数：uchar ms
 * 返回  值：无
 * 功能说明：在定时器中调用，用于监控数据接收
 ***************************************************************************************************/   
void UartRxMonitor(uchar ms) 					
{
	static uchar USART_RX_BKP = 0;  			//定义USART2_RC_BKP暂时存储诗句长度与实际长度比较
	static uchar idletmr      = 0;        		//定义监控时间
	
	if(USART_RX_CNT > 0)					//接收计数器大于零时，监控总线空闲时间
	{
		if(USART_RX_BKP != USART_RX_CNT) 	//接收计数器改变，即刚接收到数据时，清零空闲计时
		{
			USART_RX_BKP = USART_RX_CNT;  	//赋值操作，将实际长度给USART2_RX_BKP
			idletmr      = 0;               //将监控时间清零
		}
		else                              	//接收计数器未改变，即总线空闲时，累计空闲时间
		{
											//如果在一帧数据完成之前有超过3.5个字节时间的停顿，接收设备将刷新当前的消息并假定下一个字节是一个新的数据帧的开始
			if(idletmr < 3)                 //空闲时间小于3ms时，持续累加
			{
				idletmr += ms;
				if(idletmr >= 3)            //空闲时间达到3ms时，即判定为1帧接收完毕
				{
					From_Flag = 1;			//设置命令到达标志，帧接收完毕标志
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
 * 函数名称：void UartDriver(void)
 * 入口参数：无
 * 返回  值：无
 * 功能说明：串口驱动函数，检测数据帧的接收，调度功能函数，需在主循环中调用
 *           这里是主机，所以在功能调度函数里只需要得到读命令返回的数据，
 *           得到的数据根据自已的需要使用即可，这里是基于modbus协议，所以
 *           需要对照modbus协议去理解，请查阅资料里的modbus资料
 ***************************************************************************************************/   
void UartDriver(void)
{
//	unsigned char i;
	
	unsigned int crc;
	unsigned char crch,crcl;
	static unsigned char len;
	static unsigned char buf[64];
	if (From_Flag==1)            									//帧接收完成标志，即接收到一帧新数据
	{
		From_Flag = 0;           								//帧接收完成标志清零
		len       = UartRead(buf,sizeof(buf));   				//将接收到的命令读到缓冲区中
		crc       = GetCRC16(buf,len-2);       					//计算CRC校验值，除去CRC校验值
		crch=crc  >> 8;    										//crc高位
		crcl=crc  &  0xFF;										//crc低位

		if((buf[len-2] == crch) && (buf[len-1] == crcl))  		//判断CRC校验是否正确
		{
			if (buf[0] == 0x01)									//0x03 读命令
			{
				if((buf[1] == 0x03) && (buf[2] <= 0x02))  		//寄存器地址支持0x0000~0x0005
				{
					/* 通过上面的验证判断后 在这里可直接获取数据 保存在ReadDateVal中 */
					ReadDateVal = buf[3] * 256 + buf[4];		//modbus数据是16位的
					memset(buf, 0, sizeof(buf));  
         return;					
				}
			}
			/* 写命令不需要数据只需要应答即可 */
			
			/* 判断校验码正确后 无论是读还是写 都清零485忙标志，表示收到应答，释放485，可进行其它命令操作 */
//			RS485Busy = 0;										
		}
		else	memset(buf, 0, sizeof(buf)); return;   /* 如果校验值错误，执行错误处理函数 这里用串口1打印一条提示信息 也可自定义共它处理函数 */
			
	}
}

/****************************************************************************************************
 * 函数名称：void RS485_RW_Opr(uchar ucAddr,uchar ucCmd,uint ucReg,uint uiDate)
 * 入口参数：uchar ucAddr,uchar ucCmd,uint ucReg,uint uiDate
 * 			 ucAddr：从机地址
 *			 ucCmd ：功能码 03->读	06->写
 *			 ucReg ：寄存器地址
 *			 uiDate：写操作即是发送的数据 读操作即是读取数据个数
 * 返回  值：无
 * 功能说明：485读写操作函数
 ***************************************************************************************************/   
void RS485_RW_Opr(uchar ucAddr,uchar ucCmd,uint ucReg,uint uiDate)
{
	unsigned int crc;
	unsigned char crcl;
	unsigned char crch;	
	unsigned char ucBuf[8];
	
	ucBuf[0] = ucAddr;				/* 从机地址 */
	ucBuf[1] = ucCmd;				/* 命令 06 写 03 读 */
	ucBuf[2] = 0x00;//ucReg >> 8;			/* 存器地址高位 */
	ucBuf[3] = ucReg;				/* 害存器地址低位 */
	ucBuf[4] = 0x00;//uiDate >> 8;			/* 数据高8位 */
	ucBuf[5] = uiDate;				/* 数据低8位 */
	crc      = GetCRC16(ucBuf,6);   /* 计算CRC校验值 */
	crch     = crc >> 8;    		/* crc高位 */
	crcl     = crc &  0xFF;			/* crc低位 */
	ucBuf[6] = crch;				/* 校验高8位 */
	ucBuf[7] = crcl;				/* 校验低8位 */
	
	Send_Data(ucBuf,8);				/* 发送数据 */
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
	 	if(sec>1000)  //1S采集一次数据
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

/************************串口1接收********************/
void uart_isr() interrupt 4 using 2
{
  if(RI)
 {
  RI=0;
  Rec=SBUF; 
	 if(USART_RX_CNT<8)
	 {
	  	USART_RX_BUF[USART_RX_CNT] = Rec;			//记录接收到的值
	    USART_RX_CNT ++;								//接收数据增加1 
	 }
	 else {USART_RX_CNT=0; memset(USART_RX_BUF, 0, sizeof(USART_RX_BUF));return;} 
  Send_byte(Rec);	 
 }
  if(TI)  TI=0;
}




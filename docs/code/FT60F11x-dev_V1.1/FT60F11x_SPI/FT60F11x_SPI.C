// Project:  FT60F11x_SPI.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description: 此演示程序为60F11x_SPI的演示程序.该程序写入0x55到(25C64)0x13地址,然后读取0x13地址值

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//*********************************************************
#include "SYSCFG.h"
//***********************宏定义*****************************
#define  unchar     unsigned char 
#define  unint      unsigned int

#define  MISO		PA4   
#define  MOSI		PA2
#define  SCK		PA6
#define  CS			PA7

volatile unchar 	SPIReadData;
/*-------------------------------------------------
 * 函数名：interrupt ISR
 * 功能：  中断处理
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void interrupt ISR(void)
{
}
/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/	
void POWER_INITIAL (void) 
{
	OSCCON = 0B01110001;	//IRCF=111=16MHz/4=4MHz,0.25us

	INTCON = 0;  			//暂禁止所有中断
	PORTA = 0B00000000;		
	TRISA = 0B00010000;		//PA输入输出 0-输出 1-输入
							//PA4-IN PA2-OUT
	PORTC = 0B00000000; 	
	TRISC = 0B00000000;		//PC输入输出 0-输出 1-输入  
	WPUA = 0B00010000;    	//PA端口上拉控制 1-开上拉 0-关上拉
							//开PA4上拉
                            
	OPTION = 0B00001000;	//Bit3=1 WDT MODE,PS=000=1:1 WDT RATE
                             
    PSRCA = 0B11111111;  	//源电流设置最大
    PSRCC = 0B11111111; 
    PSINKA = 0B11111111;    //灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON = 0B00110000;		   	
    //Bit5: PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit4: PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit3: UCFG1<1:0>为01时此位有意义。0：禁止LVR；1：打开LVR
    //Bit2: 快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
    //Bit1: 0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
    //Bit0: 0：睡眠时停止工作：1： 睡眠时保持工作。当T2时钟不是选择指令时钟的时候
}
/*-------------------------------------------------
 * 函数名：init_25c64_io
 * 功能：  25C64初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/	
void init_25c64_io(void)
{
    CS = 1;
 	SCK = 0;
    MOSI = 0;
}
/*-------------------------------------------------
 * 函数名：SPI_RW
 * 功能：  主机输出以及输入一个字节
 * 输入：  data
 * 输出：  根据接收的data输出给从机一个字节
 --------------------------------------------------*/		  
unchar SPI_RW(unchar data)
{
   unchar i; 
   for(i=0;i<8;i++)
   {
    if(data&0x80)
      MOSI = 1;				//输出 'uchar',MSB to MOSI
    else
      MOSI = 0;
    NOP();
    data <<= 1;				//切换下一位到 MSB
    SCK = 1;				//SCK置高...
	NOP();
    if(MISO)
      data |= 0x01;
    else
      data &= 0xFE;
	NOP();
    SCK = 0;
   }
   return data;
 }
/*-------------------------------------------------
* 函数名：WriteEnable
* 功能：  写允许（将WEN置位） 
 --------------------------------------------------*/
void WriteEnable(void)
{
    CS=0;
    SPI_RW(0x06);  
    CS=1;
}
/*-------------------------------------------------
 * 函数名：WriteDisable
 * 功能：  写禁止（将WEN复位） 
 --------------------------------------------------*/
 void WriteDisable (void)
{
    CS=0;
    SPI_RW(0x04);  
    CS=1;
}
/*-------------------------------------------------
 * 函数名: SPI_ReadStatus
 * 功能：  读取25C64芯片的状态。
 * 返回值：状态寄存器数据字节
 * 注：    25C64内部状态寄存器第0位=0表示空闲，0位=1表示忙。
 --------------------------------------------------*/
unchar SPI_ReadStatus(void)
{
    unchar status=0;
    CS=0;
    SPI_RW(0x05);                 //0x05读取状态的命令字
    status = SPI_RW(0x00);        
    CS=1;                         //关闭片选
    return status;
}  
/*-------------------------------------------------
* 程序名：SPI_WriteStatus
* 功能:   写25C64芯片的状态寄存器。
*         只有BP1、BP0 (bit7、3、2)可以写、
* 注：    25c64内部状态寄存器第0位=0表示空闲，0位=1表示忙。
 --------------------------------------------------*/
void SPI_WriteStatus(unchar Status)
{
    CS=0;
    SPI_RW(0X01);  		          //0x01读取状态的命令字
    SPI_RW(Status);               //写入一个字节
    CS=1;                         //关闭片选
}
/*-------------------------------------------------
 * 程序名：SPI_Read
 * 输入:   16位的地址
 * 返回:   读取的数据
 * 说明：  从25c64指定的地址读取一个字节
 --------------------------------------------------*/
unchar SPI_Read(unint addr)
{
	unchar spidata;
    while(SPI_ReadStatus()&0x01);       //判断是否忙
    CS=0;                               //使能器件   
    SPI_RW(0x03);                       //发送读取命令   
    SPI_RW((unsigned char)((addr)>>8));
    SPI_RW((unsigned char)addr);
  	spidata = SPI_RW(0x00);             //读出数据  
    CS=1;  
  	return spidata;            
}
/*-------------------------------------------------
 * 程序名：SPI_Write
 * 输入:   地址，字节数据
 * 说明：  将一个字节写入指定的地址
 --------------------------------------------------*/
void SPI_Write(unint addr,unchar dat)
{
    while(SPI_ReadStatus()&0x01);       //判断是否忙
    WriteEnable();                      //SET WEL
    CS=0;                               //使能器件   
    SPI_RW(0x02);   				    //发送写命令 
    SPI_RW((unchar)((addr)>>8));
    SPI_RW((unchar)addr);
    
    SPI_RW(dat);
    CS=1;                               //关闭片选
	WriteDisable(); 
    while(SPI_ReadStatus()&0x01); 
}
/*-------------------------------------------------
 * 函数名: main 
 * 功能：  主函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void main()
{
	POWER_INITIAL();				        //系统初始化
	init_25c64_io();
    SPI_Write(0x0013,0x55); 
	SPIReadData = SPI_Read(0x0013); 		//读取0x13地址值 
	
	while(1)
	{
    	NOP();
	}
}

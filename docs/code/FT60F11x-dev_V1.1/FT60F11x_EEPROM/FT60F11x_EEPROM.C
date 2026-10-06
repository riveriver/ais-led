// Project:  FT60F11x_EEPROM.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description:  此演示程序为60F11x EEPROM的演示程序.把0x55写入地址0x13,再读出该值

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//*********************************************************
#include "SYSCFG.h"
//*********************宏定义******************************
#define 	unchar     	unsigned char 
volatile  	unchar 		EEReadData; 
/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/	
void POWER_INITIAL (void) 
{ 
	OSCCON = 0B01110001;	//IRCF=111=16MHz/2=8MHz,0.125us
	INTCON = 0;  			//暂禁止所有中断
    
	PORTA = 0B00000000;		
	TRISA = 0B00000000;		//PA输入输出 0-输出 1-输入
	PORTC = 0B00000000; 	
	TRISC = 0B00000000;		//PC输入输出 0-输出 1-输入  
	WPUA = 0B00000000;    	//PA端口上拉控制 1-开上拉 0-关上拉
                            
	OPTION = 0B00001000;	//Bit3=1,WDT MODE,PS=000=WDT RATE 1:1
                                 
    PSRCA = 0B11111111;		//源电流设置最大
    PSRCC = 0B11111111; 
    PSINKA = 0B11111111;    //灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON = 0B00110000;		   	
	//Bit5:PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
	//Bit4:PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
	//Bit3:UCFG1<1:0>为01时此位有意义。0：禁止LVR；1：打开LVR
	//Bit2:快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
	//Bit1:0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
	//Bit0:0：睡眠时停止工作：1： 睡眠时保持工作。当T2时钟不是选择指令时钟的时候
}
/*-------------------------------------------------
 * 函数名：interrupt ISR
 * 功能：  中断处理
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void interrupt ISR(void)
{
}
/*----------------------------------------------------
 * 函数名称：EEPROMread
 * 功能：    读EEPROM数据
 * 输入参数：EEAddr 需读取数据的地址
 * 返回参数；ReEEPROMread 对应地址读出的数据
 ----------------------------------------------------*/
unchar EEPROMread(unchar EEAddr)
{
	unchar ReEEPROMread;

	EEADR = EEAddr;    
	RD = 1;
	ReEEPROMread = EEDAT;     		//EEPROM的读数据 ReEEPROMread = EEDATA;
	return ReEEPROMread;
}
/*---------------------------------------------------- 
 * 函数名称：EEPROMwrite
 * 功能：    写数据到EEPROM
 * 输入参数：EEAddr 需要写入数据的地址
 *           Data 需要写入的数据
 * 返回参数：无
 ----------------------------------------------------*/
void EEPROMwrite(unchar EEAddr,unchar Data)
{
	GIE = 0;						//写数据必须关闭中断
	while(GIE); 					//等待GIE为0
	EEADR = EEAddr; 	 			//EEPROM的地址
	EEDAT = Data;		 			//EEPROM的写数据  EEDATA = Data;
	EEIF = 0;
	EECON1 |= 0x34;					//置位WREN1,WREN2,WREN3三个变量.
	WR = 1;							//置位WR启动编程
	while(WR);      				//等待EE写入完成
	GIE = 1;
}
/*-------------------------------------------------
 * 函数名: main
 * 功能：  主函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void main()
{
	POWER_INITIAL();				//系统初始化
	EEPROMwrite(0x13,0x55); 		//0x55写入地址0x13
    EEReadData = EEPROMread(0x13); 	//读取0x13地址EEPROM值 
    
	while(1) 
	{
		NOP();
	}
}

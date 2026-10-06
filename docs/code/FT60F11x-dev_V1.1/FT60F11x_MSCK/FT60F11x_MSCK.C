// Project:  FT60F11x_MSCK.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description: 程序中读取快时钟测量慢时钟数据

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//*******************************************************
#include "SYSCFG.h"
//***********************宏定义**************************
#define 	unint       unsigned int
 
volatile    unint      	TestBuff;
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
	OSCCON = 0B01110001;	//IRCF=111=16MHz/2=8MHz,0.125us
	INTCON = 0;				//暂禁止所有中断
	PORTA = 0B00000000;		
	TRISA = 0B00000000;		//PA输入输出 0-输出 1-输入
							
						
	PORTC = 0B00000000; 	
	TRISC = 0B00000000;		//PC输入输出 0-输出 1-输入  
								
	WPUA = 0B00000000;		//PA端口上拉控制 1-开上拉 0-关上拉
							
	OPTION = 0B00001000;	//Bit3=1 WDT MODE,PS=000=1:1 WDT RATE
                             
    PSRCA = 0B11111111;		//源电流设置最大
    PSRCC = 0B11111111; 
    PSINKA = 0B11111111;	//灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON  = 0B00110000;		   	
    //Bit5:	PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit4:	PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit3:	UCFG1<1:0>为01时此位有意义。0：禁止LVR；1：打开LVR
    //Bit2:	快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
    //Bit1:	0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
    //Bit0:	0：睡眠时停止工作：1： 睡眠时保持工作。当T2时钟不是选择指令时钟的时候
}
/*----------------------------------------------------
* 函数名称： DelayUs
* 功能：     短延时函数 --16M-2T--大概快1%左右.
* 输入参数： Time 延时时间长度 延时时长Time Us
* 返回参数： 无 
 ----------------------------------------------------*/
void DelayUs(unsigned char Time)
{
	unsigned char a;
	for(a=0;a<Time;a++)
	{
		NOP();
	}
}                  
/*----------------------------------------------------
* 函数名称：DelayMs
* 功能：    短延时函数
* 输入参数：Time 延时时间长度 延时时长Time ms
* 返回参数：无 
 ----------------------------------------------------*/
void DelayMs(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<5;b++)
		{
		 	DelayUs(197);    //快1%
		}
	}
}

/*-------------------------------------------------
* 函数名: SlowTimeTest
* 功能：  快时钟测量慢时钟
* 输入：  无
* 输出：  慢时钟时钟测量值TestTime
*		  不开平均模式慢时钟频率=16M/TestTime(2T)
*		  开平均模式慢时钟频率 = 16M/TestTime/4(2T)
 --------------------------------------------------*/
unint SlowTimeTest()
{
	unint TestTime;
	OSCCON = 0B01110001;		//IRCF=111=16MHz/2=8MHz,0.125us 
								//IRCF = 111,SCS = 1.
	TMR2ON = 1;					//开定时器2
	CKMIF = 0;			    	//清标志位
	CKMAVG = 0;					//关闭平均模式 
								//注:打开平均模式输出数据为四个周期的时钟数(单周期*4)
	CKCNTI = 1; 				//使能快时钟测量位,开始测量
	while(!CKMIF);
	CKMIF = 0;
	TestTime = SOSCPRH << 8;
	TestTime = TestTime + SOSCPRL;
	return TestTime;
}
/*-------------------------------------------------
* 函数名:  main
* 功能：  主函数
* 输入：  无
* 输出：  无
 --------------------------------------------------*/
void main(void)
{
	POWER_INITIAL();				//系统初始化
    
	while(1)
	{
	 
		TestBuff = SlowTimeTest();  //时钟测量值
									//32768该数值≈488(不开平均模式-单周期)
                                    //慢时钟= TestBuff/16(kHz)
		NOP();
		NOP();
		NOP();
		DelayMs(200); 				//延时200ms
	}
}
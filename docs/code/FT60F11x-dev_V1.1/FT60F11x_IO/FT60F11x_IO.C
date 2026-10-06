// Project:  FT60F11x_IO.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description: 当DemoPortIn悬空或者高电平时，DemoPortOut输出50Hz占空比50%的波形，当DemoPortIn接地时，DemoPortOut输出高电平

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//*********************************************************
#include "SYSCFG.h"
//***********************宏定义****************************
#define  DemoPortOut	PA4   
#define  DemoPortIn		PA6
/*-------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/	
void POWER_INITIAL (void) 
{ 
	OSCCON = 0B01110001;	//IRCF=111=16MHz/2T=8MHz,0.125us
	INTCON = 0;  			//暂禁止所有中断
	PORTA = 0B00000000;		
	TRISA = 0B01000000;		//PA输入输出 0-输出 1-输入
							//PA4-OUT PA6-IN
	PORTC = 0B00000000; 	
	TRISC = 0B00000000;		//PC输入输出 0-输出 1-输入  
								
	WPUA = 0B01000000;    	//PA端口上拉控制 1-开上拉 0-关上拉
							//开PA6上拉
	WPUC = 0B00000000;    	//PC端口上拉控制 1-开上拉 0-关上拉
							//60系列PC口无上拉	
                            
	OPTION = 0B00001000;	//Bit3=1,WDT MODE,PS=000=WDT RATE 1:1                             
    PSRCA = 0B11111111;    	//源电流设置最大
    PSRCC = 0B11111111; 
    PSINKA = 0B11111111;    //灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON = 0B00110000;		   	
	//Bit5:	PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
	//Bit4:	PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
	//Bit3:	UCFG1<1:0>为01时此位有意义。0：禁止LVR；	 1：打开LVR
	//Bit2:	快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
	//Bit1:	0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
	//Bit0:	0：睡眠时停止工作：1： 睡眠时保持工作。
    //		当T2时钟不是选择指令时钟的时
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
 * 函数名称：DelayUs
 * 功能：    短延时函数 --16M-2T--大概快1%左右.
 * 输入参数：Time延时时间长度 延时时长Time Us
 * 返回参数：无 
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
 * 输入参数：Time延时时间长度 延时时长Time ms
 * 返回参数：无 
 ----------------------------------------------------*/
void DelayMs(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<5;b++)
		{
		 	DelayUs(197); 	//快1%
		}
	}
}
/*-------------------------------------------------
 *  函数名: main 
 * 功能：   主函数
 *  输入：  无
 *  输出：  无
 --------------------------------------------------*/
void main(void)
{
	POWER_INITIAL();			//系统初始化
    
	while(1)
	{
		DemoPortOut = 1; 		
		DelayMs(10);     		//10ms
        
		if(DemoPortIn == 1) 	//判断输入是否为高电平 
		{
			DemoPortOut = 0;
		}
		DelayMs(10); 
	}
}
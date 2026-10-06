// Project:  FT60F11x_SLEEP.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description:  此程序为FT60F11x-SLEEP睡眠演示程序，上电之后led1、led2同时置高约4s，然后置低，进入睡眠。测试FT60F11x的睡眠功耗：

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//*********************************************************
#include "SYSCFG.h"
//***********************宏定义****************************
#define  led1			PA4
#define  led2			PA5
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
	OSCCON = 0B01110001;	//IRCF=111=16MHz/4T=4MHz,0.25us

	INTCON = 0;  			//暂禁止所有中断
	PORTA = 0B00000000;		
	TRISA = 0B00000000;		//PA输入输出 0-输出 1-输入
							//PA4,PA5-OUT 
	PORTC = 0B00000000; 	
	TRISC = 0B00000000;		//PC输入输出 0-输出 1-输入  
	WPUA = 0B00000000;    	//PA端口上拉控制 1-开上拉 0-关上拉
	OPTION = 0B00001000;	//Bit3=1 WDT MODE,PS=000=1:1 WDT RATE
                             
    PSRCA = 0B11111111;    //源电流设置最大
    PSRCC = 0B11111111; 
    PSINKA = 0B11111111;    //灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON  = 0B00110000;		   	
    //Bit5: PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit4: PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit3: UCFG1<1:0>为01时此位有意义。0：禁止LVR；1：打开LVR
    //Bit2: 快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
    //Bit1: 0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
    //Bit0: 当T2时钟不是选择指令时钟的时候
    //		0：睡眠时停止工作：1： 睡眠时保持工作
}
 /*-------------------------------------------------
* 函数名称：DelayUs
* 功能：    短延时函数 --16M-4T--大概快1%左右.
* 输入参数：Time 延时时间长度 延时时长Time*2 Us
* 返回参数：无 
 -------------------------------------------------*/
void DelayUs(unsigned char Time)
{
	unsigned char a;
	for(a=0;a<Time;a++)
	{
		NOP();
	}
}                  
/*------------------------------------------------- 
* 函数名称：DelayMs
* 功能：    短延时函数
* 输入参数：Time 延时时间长度 延时时长Time ms
* 返回参数：无 
 -------------------------------------------------*/
void DelayMs(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<5;b++)
		{
		 	DelayUs(98);	//快1%
		}
	}
}
/*------------------------------------------------- 
* 函数名称：DelayS
* 功能：   短延时函数
* 输入参数：Time 延时时间长度 延时时长Time S
* 返回参数：无 
 -------------------------------------------------*/
void DelayS(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<10;b++)
		{
		 	DelayMs(100); 
		}
	}
}/*-------------------------------------------------
* 函数名：main 
* 功能：  主函数
* 输入：  无
* 输出：  无
 --------------------------------------------------*/
void main(void)
{
	POWER_INITIAL();		//系统初始化
	led1 = 1;
    led2 = 1;
    DelayS(4);
    led1 = 0;
    led2 = 0;

    while(1)
	{
    	CLRWDT();  		    //清看门狗
		NOP();
     	SLEEP(); 
		NOP();
	}
}
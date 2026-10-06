// Project:  FT60F11x_INT.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description: 程序中DemoPortOut(PA4)输出100帧50Hz的占空比为50%的方波后, MCU进入睡眠, 等待外部中断的发生；当外部中断触发后，重复以上流程;

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//*********************************************************
#include "SYSCFG.h"
//***********************宏定义*****************************
#define  unchar			unsigned char 
#define  DemoPortOut	PA4   

unchar	FCount;
/*-------------------------------------------------
 * 函数名：interrupt ISR
 * 功能：  中断处理函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void interrupt ISR(void)	
{ 
	//PA2外部中断处理
	if(INTE && INTF)		
	{
		INTF = 0;  			//清PA2 INT 标志位
		INTE = 0;  			//暂先禁止PA2中断
	 
	}
} 
/*----------------------------------------------------
 * 函数名：POWER_INITIAL
 * 功能：  上电系统初始化
 * 输入：  无
 * 输出：  无
 ----------------------------------------------------*/	
void POWER_INITIAL (void) 
{ 
	OSCCON = 0B01110001;	//IRCF=111=16MHz/2=8MHz,0.125us
	INTCON = 0;  			//暂禁止所有中断
	PORTA  = 0B00000100;		
	TRISA  = 0B00000100;	//PA输入输出 0-输出 1-输入
							//PA4->输出
						
	PORTC  = 0B00000000; 	
	TRISC  = 0B00000100;	//PC输入输出 0-输出 1-输入  
								
	WPUA   = 0B00000100;    //PA端口上拉控制 1-开上拉 0-关上拉
							//开PA2上拉
	WPUC   = 0B00000000;    //PC端口上拉控制 1-开上拉 0-关上拉
							//60系列PC口无上拉	
                            
	OPTION = 0B00001000;	//Bit3=1 WDT MODE,PS=000=1:1 WDT RATE
                             
    PSRCA  = 0B11111111;	//源电流设置最大
    PSRCC  = 0B11111111; 
    PSINKA = 0B11111111;    //灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON  = 0B00110000;		   	
    //Bit5:	PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit4:	PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit3:	UCFG1<1:0>为01时此位有意义。0：禁止LVR； 	 1：打开LVR
    //Bit2:	快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
    //Bit1:	0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
    //Bit0:	当T2时钟不是选择指令时钟的时候
    //		0: 睡眠时停止工作； 1: 睡眠时保持工作。	
}
/*---------------------------------------------------- 
 * 函数名称：DelayUs
 * 功能：    短延时函数 --16M-2T--大概快1%左右.
 * 输入参数：Time 延时时间长度 延时时长Time Us
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
		 	DelayUs(197);	//快1%
		}
	}
}
/*-------------------------------------------------
 * 函数名: INT_INITIAL 
 * 功能：  中断初始化函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void INT_INITIAL(void)
{
	TRISA2 =1;								//SET PA2 INPUT
	IOCA2 =0;								//禁止PA2电平变化中断
	INTEDG =1;								//OPTION,INTEDG=1;PA2 INT 为上升沿触发 
	INTF =0;								//清PA2 INT中断标志位
	INTE =1;								//使能PA2 INT中断
}
/*-------------------------------------------------
 * 函数名：main 
 * 功能：  主函数
 * 输入：  无
 * 输出：  无
 --------------------------------------------------*/
void main()
{
	POWER_INITIAL();						//系统初始化
	
	while(1)
	{
		for(FCount=0;FCount<100;FCount++)	//输出100次波形	
		{
			DemoPortOut = 1; 				
			DelayMs(10);  					//10ms 
			DemoPortOut = 0;
			DelayMs(10); 
		}
		INT_INITIAL();						//初始化外部中断
		GIE = 1;							//开总中断
		SLEEP(); 							//睡眠
	}
}
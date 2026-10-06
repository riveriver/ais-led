// Project:  FT60F11x_TIMER2.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description: 当DemoPortIn悬空或者高电平时,DemoPortOut输出5kHz占空比50%的波形-TIMER2实现当DemoPortIn接地时,DemoPortOut输出高电平.关定时器

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//**********************************************************
#include "SYSCFG.h"
//***********************宏定义*****************************
#define  DemoPortOut	PA4   
#define  DemoPortIn		PA6
/*-------------------------------------------------
 *  函数名：interrupt ISR
 *	功能：  定时器2中断处理程序
 *  输入：  无
 *  输出：  无
 --------------------------------------------------*/
void interrupt ISR(void)			
{ 
	//定时器2的中断处理
	if(TMR2IE && TMR2IF)			//100us中断一次 = 5kHz
	{
		TMR2IF = 0;

		DemoPortOut = ~DemoPortOut; //翻转电平
	} 
} 
/*-------------------------------------------------
 *  函数名：POWER_INITIAL
 *	功能：  上电系统初始化
 *  输入：  无
 *  输出：  无
 --------------------------------------------------*/	
void POWER_INITIAL (void) 
{
	OSCCON = 0B01110001;	//IRCF=111=16MHz/2=8MHz,0.125us
	INTCON = 0;  			//暂禁止所有中断
	PORTA = 0B00000000;		
	TRISA = 0B01000000;		//PA输入输出 0-输出 1-输入
							//PA4-OUT RA6-IN
						
	PORTC = 0B00000000; 	
	TRISC = 0B11111111;		//PC输入输出 0-输出 1-输入  
								
	WPUA = 0B01000000;    	//PA端口上拉控制 1-开上拉 0-关上拉
							//开PA6上拉
	OPTION = 0B00001000;	//Bit3=1 WDT MODE,PS=000=WDT RATE 1:1 
                             
    PSRCA = 0B11111111;		//源电流设置最大
    PSRCC = 0B11111111; 
    PSINKA = 0B11111111;    //灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON = 0B00110000;		   	
    //Bit5:   PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit4:   PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit3:   UCFG1<1:0>为01时此位有意义。0：禁止LVR；1：打开LVR
    //Bit2:   快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
    //Bit1:	  0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
	//Bit0:	  当T2时钟不是选择指令时钟的时候
	//		  0：睡眠时停止工作：1： 睡眠时保持工作。	
}
/*-------------------------------------------------
 * 函数名称： TIMER2_INITIAL
 * 功能：	  初始化设置定时器2 
 * 设置TMR2定时时长=1/系统时钟频率*2*预分频值*后分频值*PR2
 *				   =(1/16000000)*2*4*1*200=100us
 -------------------------------------------------*/
void TIMER2_INITIAL (void) 
{    
    T2CON0  = 0B00000001; 	//T2预分频1:4，后分频1：1
    //Bit[6:3]: 定时器2输出后分频比 0000-1:1
    //Bit2:		定时器2输出是能位   0-关闭 1-使能
    //Bit[1:0]:	定时器2预分频比  01-1:4
    
	T2CON1  = 0B00000000;	//T2时钟来自系统时钟,PWM1连续模式
	//Bit4: PWM单脉冲模式选择 0-连续 1-单脉冲
    //Bit3: PWM蜂鸣器模式选择 0-PWM模式 1：蜂鸣器模式	
    //Bit[2:0]:Timer2时钟源选择 000-指令时钟
    						
    TMR2H = 0;				//TMR2赋值
    TMR2L = 0;
    
	PR2H = 0; 				//PR赋值
	PR2L = 200;	  
    
	TMR2IF = 0;				//清TMER2中断标志
	TMR2IE = 1;				//使能TMER2的中断（配置成timer定时器时不注释）
	TMR2ON = 1;				//使能TMER2启动
	PEIE = 1;    			//使能外设中断
	GIE = 1;   				//使能全局中断
}
/*-------------------------------------------------
 *  函数名: main 
 *	功能：  主函数
 *  输入：  无
 *  输出：  无
 --------------------------------------------------*/
void main()
{
	POWER_INITIAL();		//系统初始化
	TIMER2_INITIAL();  		//初始化T2
	
	while(1)
	{ 
		if(DemoPortIn) 		//判断输入是否为高电平 
		{
			TMR2IE = 1; 	//开定时器2
		}
		else
		{
			TMR2IE = 0; 	//关定时器2
			DemoPortOut = 1;
		}  
	}
}
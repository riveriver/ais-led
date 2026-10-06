// Project:  FT60F11x_IR_SEND.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description: 演示程序中,IR红外是采用6122协议，起始信号是9ms低电平，到4.5ms高电平，再到低8位用户识别码，到高8位的用户识别码，8位数据码，8位数据码的反码。SendIO（PA4）定时（5秒钟）发送一次，接收端收到遥控器发过来的数据后，校验数据互为补码，LED会开关。

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//********************************************************
#include	"SYSCFG.h"
//**********************宏定义****************************
#define  uchar	unsigned char 
#define  uint	unsigned int

#define  IRSendIO		PA4  					// 串口的发送脚

#define IRSend_HIGH_1	1  						// 560uS
#define IRSend_LOW_1    3 						// 1680uS

#define IRSend_HIGH_0   1  						// 560uS
#define IRSend_LOW_0    1  						// 560uS

#define IRSend_PIN_1	T0IE = 1  				// 发送数据  开启定时器0
#define IRSend_PIN_0	T0IE = 0 				// 关闭定时器0

#define Status_NOSend	0			            // 不发送的状态
#define Status_Head		1			            // 发送引导码的状态
#define Status_Data		2			            // 发送数据的状态

uchar IRSendStatus;								// 发送状态，是发送引导码还是数据
uchar IRSendData;								// 发送的数据中转变量
uchar  TxBit=0,TxTime=0; 
uchar SendBit = 0;
uchar level0,level1;							// 一位数据里发送与关闭的时间值
bit SendLastBit = 0;
uchar SaveLastBit = 0;
uint SYSTime5S = 0;								// 系统时间，5S发送一次

uchar IRData[4] = {0x00,0xff,0x40,0xBf};		// 需要发送的4个数据
/*----------------------------------------------------
* 函数名：POWER_INITIAL
* 说明：初始化单片机
* 输入：无
* 输出：无
 ----------------------------------------------------*/
void POWER_INITIAL(void)
{
	OSCCON = 0B01110001;	//IRCF=111=16MHz/2=8MHz,0.125us
					 		//Bit0=1,系统时钟为内部振荡器

	INTCON = 0;  			//暂禁止所有中断
	PORTA  = 0B00000000;		
	TRISA  = 0B11101111;	//PA输入输出 0-输出 1-输入
							//PA4->输出
						
	PORTC  = 0B00000000; 	
	TRISC  = 0B11111111;	//PC输入输出 0-输出 1-输入  
								
	WPUA   = 0B00000000;    //PA端口上拉控制 1-开上拉 0-关上拉
							
	WPUC   = 0B00000000;    //PC端口上拉控制 1-开上拉 0-关上拉
							//60系列PC口无上拉	
                            
	OPTION = 0B00001000;	//Bit3=1 WDT MODE,PS=000=1:1 WDT RATE
                             
    PSRCA  = 0B11111111;  	//源电流设置最大
    PSRCC  = 0B11111111; 
    PSINKA = 0B11111111;    //灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON  = 0B00110000;		   	
    //Bit5: PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit4: PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit3: UCFG1<1:0>为01时此位有意义。0：禁止LVR；1：打开LVR
    //Bit2: 快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
    //Bit1: 0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
    //Bit0: 0：睡眠时停止工作：1： 睡眠时保持工作。当T2时钟不是选择指令时钟的时候	
}

/*----------------------------------------------------
* 函数名称：TIMER0_INITIAL
* 功能：初始化设置定时器v
* 相关寄存器：T0CS T0CS T0SE PSA 
* 说明：38KHz发生器，1000000/38000=26.3uS .由于定时太短，频繁进定时器，时间有一定的
* 误差，239并不是直接算出来的， 是示波器看的。
* 设置TMR0定时时长=(1/系统时钟频率)*指令周期*预分频值*26
*				  =(1/16000000)*4*2*26=13us
 ----------------------------------------------------*/
void TIMER0_INITIAL (void)  
{
	OPTION = 0B00000000;    
    //Bit5 T0CS Timer0时钟源选择 
	//1-外部引脚电平变化T0CKI 0-内部时钟(FOSC/2)
	//Bit4 T0CKI引脚触发方式 1-下降沿 0-上升沿
	//Bit3 PSA 预分频器分配位 0-Timer0 1-WDT 
	//Bit2:0 PS2 8个预分频比 000 - 1:2
	TMR0 = 239; 
    T0IF = 0;				//清空T0软件中断
}

/*----------------------------------------------------
* 函数名：TIMER2_INITIAL
* 功能：  初始化设置定时器2 配置成PWM
* 设置：  TMR2输出比较值定时=(1/系统时钟频率)*4*预分频值*后分频值*PR2
*							  =(1/16000000)*4*4*4*141
*							  =564us
----------------------------------------------------*/
void Timer2Inital(void) 
{
	T2CON0  = 0B00011001; 			//T2预分频1:4，后分频1：4
    //Bit[7:0]:	无意义； 1：把PR2/P1xDTy缓冲值分别更新到PR2寄存器和P1xDTy_ACT
    //Bit[6:3]:	定时器2输出后分频比选择 0000: 1:1;0001: 1:2;……1:16
    //Bit[2:0]:	关闭定时器2；1：打开定时器2
    //Bit[1:0]:	定时器2预分频选择 00:1;01:4;1x:16
    
	T2CON1  = 0B00000000;		   //T2时钟来自系统时钟,PWM1连续模式
	//Bit4:   	PWM模式选择
    //		  	0:连续模式；1：单脉冲模式
    //Bit3:   	0:PWM模式；1：蜂鸣器模式	
    //Bit[2:0]	Timer2时钟源选择
    //		  	000：指令时钟；
    //		  	001：系统时钟；
    //		  	010：HIRC的2倍频；
    //		  	100：HIRC；
    //		  	101：LIRC
    						
    TMR2H = 0;					//定时器2计数寄存器
    TMR2L = 141;
    
    
	PR2H = 0; 					//周期=(PR+1)*Tt2ck*TMR2预分频(蜂鸣器模式周期*2)
	PR2L = 141;	  
    
    P1ADTH = 0;					//脉宽=P1xDT*Tt2ck*TMR2预分频(蜂鸣器模式没用到)
    P1ADTL = 50;
    
    P1OE = 0B00000000;			//充许P1A0输出PWM（配置成timer定时器时这位清零）
    //Bit7: 	0:禁止P1C输出到管脚;1:充许P1C输出到管脚
    //Bit6: 	0:禁止P1B输出到管脚;1:充许P1B输出到管脚
    //Bit[5:0]:	0:禁止P1Ax输出到管脚;1:充许P1Ax输出到管脚
    
    P1POL = 0B00000000;			//高电平有效
    //Bit7: 	0:P1C高电平有效;1:P1C低电平有效
    //Bit6: 	0:P1B高电平有效;1:P1B低电平有效
    //Bit[5:0]: 0:P1Ax高电平有效;1:P1Ax低电平有效
    
    P1CON = 0B00000000;
    //Bit7:		PWM1 重启使能位
	//			1 = 故障刹车时，P1BEVT位在退出关闭事件时自动清零，PWM1自动重启
	//			0 = 故障刹车时，必须用软件将P1BEVT清零以重启PWM1
    //Bit[6:0]:	PWM1死区时间设置
	//			P1DCn = 预定MPWM信号应转变为有效与PWM信号实际转为有效之间的T2CK周期数
        
    MSCON = 0B00110000;		    //Bit0: 0:T2睡眠时停止工作	
    //Bit5:	PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit4:	PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit3:	UCFG1<1:0>为01时此位有意义。0：禁止LVR；1：打开LVR
    //Bit2:	快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
    //Bit1:	0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
    //Bit0:	0：睡眠时停止工作：1： 睡眠时保持工作。当T2时钟不是选择指令时钟的时候
    
	TMR2IF = 0;					//清TMER2中断标志
	TMR2IE = 1;					//使能TMER2的中断（配置成timer定时器时不注释）
	TMR2ON = 1;					//使能TMER2启动
	PEIE = 1;    				//使能外设中断
	GIE = 1;   					//使能全局中断
}
/*-------------------------------------------------
* 函数名：SendCtrl
* 功能：  发送数据函数
* 输入：  无
* 输出：  无
 --------------------------------------------------*/
void SendCtrl(void)
{

	if (IRSendStatus == Status_NOSend)			// 不发送的状态
	{        
		IRSend_PIN_0;
        SendBit = 0;
		TxTime = 0;
        
	}	 
	else if (IRSendStatus == Status_Head)		// 发送引导码
	{
    	TxTime++;
		if (TxTime < 17)   						// 发送9mS信号
		{
			IRSend_PIN_1;
		}
        else if (TxTime < 24)   				// 4.5mS不发送
		{
			IRSend_PIN_0;
		}
		else
		{
			TxTime = 0;
            IRSendStatus = Status_Data;
		}
        IRSendData = IRData[0];
        TxBit = 0x01;
	}
	else if(IRSendStatus == Status_Data)		// 发送数据
	{
		if (IRSendData & TxBit)  				// 1，是1:3的时间
		{
			level1 = IRSend_HIGH_1;
			level0 = IRSend_LOW_1;
		}
		else									// 0，是1:1的时间
		{
			level1 = IRSend_HIGH_0;
			level0 = IRSend_LOW_0;
		}
		TxTime++;
		if (TxTime <= level1)  					// 发送信号
		{
			IRSend_PIN_1;
		}
		else if (TxTime <= (level0+level1)) 	// 不发送信号
		{
			IRSend_PIN_0;
		}
		else if (SendBit < 4)					// 发送4位数据未完成
		{
			TxTime = 1;
            IRSend_PIN_1;
            SaveLastBit = IRSendData & TxBit;
			TxBit <<= 1;
			if (TxBit == 0x00)  				// 发送完一个字节
			{
				TxBit = 0x01;
                SendBit++;
                IRSendData = IRData[SendBit];
				if (SendBit > 3)   				// 最后一位要注意，因为发送完了还要有一个脉冲
				{
                    SendLastBit = 1;
				}
			}
		}
        else									// 数据完成了，要补脉冲
        {
        	if(SendLastBit)
 		   	{
		    	TxTime++;
		    	if(SaveLastBit)
		        {
		        	if(TxTime < 3)
		            {
		            	IRSend_PIN_0;
		            }
		            else if(TxTime < 4)
		            {
		            	IRSend_PIN_1;
		            }
		            else
		            {
		            	IRSend_PIN_0;
						IRSendStatus = Status_NOSend;
		                IRSend_PIN_0;
		                SendLastBit = 0;
                        TxBit = 0;
                        TxTime = 0;
		            }
		        }
		        else
		        {
		        	if(TxTime < 5)
		            {
		            	IRSend_PIN_0;
		            }
		            else if(TxTime < 6)
		            {
		            	IRSend_PIN_1;
		            }
		            else
		            {
		            	IRSend_PIN_0;
						IRSendStatus = Status_NOSend;
		                IRSend_PIN_0;
		                SendLastBit = 0;
                        TxBit = 0;
                        TxTime = 0;
		            }
		        }
		    }
        }
	}
}
/*-------------------------------------------------
* 函数名：interrupt ISR
* 功能：  中断处理，包括定时器0中断和外部中断
* 输入：  无
* 输出：  无
 --------------------------------------------------*/
void interrupt ISR(void)			
{ 
  //定时器0的中断处理
	if(T0IE && T0IF)				//13us
	{
		TMR0 = 239;					//注意:对TMR0重新赋值TMR0在两个周期内不变化	 
		T0IF = 0;    
		IRSendIO = ~IRSendIO; 		//翻转电平  产生38KHz信号
	} 
    //定时器2的中断处理
	if(TMR2IE && TMR2IF)			//560us中断一次 红外每一位都是560uS的倍数
	{
		TMR2IF = 0;
        SendCtrl();
        SYSTime5S++;
        //IRSendIO = ~IRSendIO; 
   	}
} 


/*-------------------------------------------------
* 函数名：main
* 功能：  主函数
* 输入：  无
* 输出：  无
 --------------------------------------------------*/
void main(void)
{
	POWER_INITIAL();
    TIMER0_INITIAL();
    Timer2Inital();
    GIE = 1; 							//开中断
    while(1)
    {
		if(SYSTime5S >10000)			//每隔5S发射一次
        {
        	SYSTime5S = 0;
            IRSendStatus = Status_Head;
        }
    }
}

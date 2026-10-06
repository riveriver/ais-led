// Project:  FT60F11x_UART.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description: 演示程序中波特率为9600，RXIO（PA2）每次收到外部串口发过来的数据后，TXIO(PA4)把收到的数据再发送出去。收起始位时是用电平变化中断识别，后面就关闭电平变化中断了。 

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   修改文件头

//*********************************************************
#include	"SYSCFG.h"
//**********************宏定义*****************************
#define  uchar     unsigned char 

#define  TXIO		PA4  	//串口的发送脚
#define  RXIO		PA2  	//串口的接收脚

#define  Bord		49 		//通过定时器提供波特率
uchar RXFLAG = 0;
uchar ReadAPin;
/*-------------------------------------------------
* 函数名: POWER_INITIAL
* 功能：  MCU初始化函数
* 输入：  无
* 输出：  无
 --------------------------------------------------*/
void POWER_INITIAL(void)
{
	OSCCON = 0B01110001;	//IRCF=111=16MHz/4=4MHz,0.25us

	INTCON = 0;  			//暂禁止所有中断
	PORTA  = 0B00000000;		
	TRISA  = 0B00000100;	//PA输入输出 0-输出 1-输入
							//PA4-OUT PA2-IN
	PORTC  = 0B00000000; 	
	TRISC  = 0B00000000;	//PC输入输出 0-输出 1-输入  
	WPUA   = 0B00000000;    //PA端口上拉控制 1-开上拉 0-关上拉
							
	OPTION = 0B00001000;	//Bit3=1 WDT MODE,PS=000=1:1 WDT RATE
					 		//Bit7(PAPU)=0 ENABLED PULL UP PA
                             
    PSRCA  = 0B11111111;	//源电流设置最大
    PSRCC  = 0B11111111; 
    PSINKA = 0B11111111;    //灌电流设置最大
    PSINKC = 0B11111111;
                      
    MSCON  = 0B00110000;		   	
    //Bit5:   PSRCAH4和PSRCA[4]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit4:   PSRCAH3和PSRCA[3]共同决定源电流。00：4mA; 11: 33mA; 01、10:8mA
    //Bit3:   UCFG1<1:0>为01时此位有意义。0：禁止LVR；1：打开LVR
    //Bit2:   快时钟测量慢周期的平均模式。0：关闭平均模式；1：打开平均模式
    //Bit1:	  0：关闭快时钟测量慢周期；1：打开快时钟测量慢周期
	//Bit0:	  当T2时钟不是选择指令时钟的时候
	//		  0：睡眠时停止工作：1： 睡眠时保持工作。	
}

/*----------------------------------------------------
* 函数名：TIMER0_INITIAL
* 功能：  初始化设置定时器
* 输入：  无
* 输出：  无
* 说明:	设置TMR0定时时长=(1/系统时钟频率)*指令周期*预分频值*208
* 						=(1/16000000)*4*2*208=104us                    
 ----------------------------------------------------*/
void TIMER0_INITIAL (void)  
{
	OPTION = 0B00000000;    
    //Bit5:		T0CS Timer0时钟源选择 
	//			1-外部引脚电平变化T0CKI 0-内部时钟(FOSC/2)
	//Bit4:		T0CKI引脚触发方式 1-下降沿 0-上升沿
	//Bit3:		PSA 预分频器分配位 0-Timer0 1-WDT 
	//Bit[2:0]:	PS2 8个预分频比 000 - 1:2
	TMR0 = Bord; 
    T0IF = 0;				//清空T0软件中断
}

/*-------------------------------------------------
* 函数名: PA2_Level_Change_INITIAL
* 功能：  PA端口(PA2)电平变化中断初始化
* 输入：  无
* 输出：  无
--------------------------------------------------*/
void PA2_Level_Change_INITIAL(void)
{
	TRISA2 =1; 								//SET PA2 INPUT
	ReadAPin = PORTA;						//清PA电平变化中断
	PAIF =0;   								//清PA INT中断标志位
    IOCA2 =1;  								//使能PA2电平变化中断
	PAIE =1;   								//使能 PA INT中断
	//GIE =1;    							//使能全局中断
}

/*-------------------------------------------------
* 函数名： interrupt ISR
* 功能：  中断处理，包括定时器0中断和外部中断
* 说明：  定时器产生104uS中断，对应9600的波特率 1000000÷9600=104
 --------------------------------------------------*/
void interrupt ISR(void)			        
{ 
   
	//定时器0的中断处
	if(T0IE && T0IF)						//104us
	{
		TMR0 = Bord;						//注意:对TMR0重新赋值TMR0在两个周期内不变化
		 
		T0IF = 0;
        T0IE = 0;
	} 
    
    //PA电平变化中断
	if(PAIE && PAIF)		
    {
		ReadAPin = PORTA; 					//读取PORTA数据清PAIF标志
		PAIF = 0;  							//清PAIF标志位
		if(RXIO == 0)
        {
        	PAIE = 0;  						//暂先禁止PA电平变化中断
			IOCA2 =0;  						//禁止PA2电平变化中断
            RXFLAG = 1;
        } 
    }
} 
/*-------------------------------------------------
* 函数名： WaitTF0
* 功能：  查询定时器溢出后，在中断里关闭定时器后，再次打开定时器
* 输入：  无
* 输出：  无
 --------------------------------------------------*/
void WaitTF0( void )
{
     while(T0IE);
     T0IE=1;
}
/*-------------------------------------------------
* 函数名： WByte
* 功能：  UART发送一个字节
* 输入：  input
* 输出：  无
 --------------------------------------------------*/
void WByte(uchar input)
{
	//发送起始位
	uchar i=8;
	TXIO = 1;
	TMR0 = Bord;
	T0IE = 1;  
	WaitTF0(); 
	TXIO=0;
	WaitTF0();
	                                        //发送8位数据位
	while(i--)
	{
		if(input&0x01) 						//先传低位
		{
			TXIO=1;
		}
		else
		{
			TXIO = 0;
		}    
		WaitTF0();
		input=input>>1;
	}
	//发送结束位
	TXIO=(bit)1;
	T0IE=0;
} 
/*-------------------------------------------------
* 函数名：RByte
* 功能：  UART接收一个字节
* 输入：  无
* 输出：  Output
 --------------------------------------------------*/
uchar RByte()
{
	uchar Output=0;
	uchar i=8;
	T0IE=1;                          		//启动Timer0
	TMR0 = Bord;
	WaitTF0();
	T0IE=1;                          		//启动Timer0
	TMR0 = Bord;
	WaitTF0();                     			//等过起始位
	                                        //发送8位数据位
	while(i--)
	{
		Output >>=1;
		if(RXIO) 
        {
        	Output   |=0x80;      			//先收低位
        }
		WaitTF0();                 			//位间延时
	}
	T0IE=0;                          		//停止Timer0
	return Output;
}
/*-------------------------------------------------
* 函数名：main
* 功能：  主函数
* 输入：  无
* 输出：  无
 --------------------------------------------------*/
void main(void)
{
	uchar rdata = 0;
	POWER_INITIAL();
    TIMER0_INITIAL();
    PA2_Level_Change_INITIAL();
    GIE = 1; 								//开中断
	T0IE = 1;								//开定时器/计数器0中断
    while(1)
    {
        if(RXFLAG)							//外部中断下降沿触发了
        {
        	rdata = RByte();
            WByte(rdata);
        
            IOCA2 =1;  						//使能PA2电平变化中断
			PAIE =1;   						//使能PA INT中断
            RXFLAG = 0;
        }
    }
}

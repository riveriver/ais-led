#include "SYSCFG.h"

/* 五组灯的GPIO映射；PA0、PA1同时复用为ISP烧录引脚。 */
#define LedGroup1 PA6
#define LedGroup2 PA0
#define LedGroup3 PA1
#define LedGroup4 PA2
#define LedGroup5 PC2

/*
 * 系统与GPIO初始化。
 * LED采用高电平点亮，所有灯组在初始化阶段保持低电平熄灭。
 */
void POWER_INITIAL (void)
{
	/* 使用16 MHz内部高速振荡器；2T指令周期对应8 MIPS。 */
	OSCCON = 0B01110001;

	/* 关闭所有中断并清除中断控制状态，本程序采用轮询延时。 */
	INTCON = 0;

	/* 先把PORTA输出锁存器清零，再将PA口配置为输出。 */
	PORTA = 0B00000000;
	TRISA = 0B00000000;

	/* PORTC初始输出低电平；PC3为输入，其余PC引脚为输出。 */
	PORTC = 0B00000000;
	TRISC = 0B00001000;

	/* PA口不使用内部上拉；PC3启用内部上拉，避免输入悬空。 */
	WPUA = 0B00000000;
	WPUC = 0B00001000;

	/* 预分频器分配给WDT，分频比设置为1:1。 */
	OPTION = 0B00001000;

	/* 设置PORTA、PORTC的源电流和灌电流驱动档位。 */
    PSRCA = 0B11111111;
    PSRCC = 0B11111111;
    PSINKA = 0B11111111;
    PSINKC = 0B11111111;

	/* PA3、PA4选择高源电流档位，其余扩展控制位保持关闭。 */
    MSCON = 0B00110000;
}

/* 中断服务函数预留；当前程序未启用中断。 */
void interrupt ISR(void)
{
}

/* 基于NOP的软件微秒级延时，适用于当前16 MHz/2T时钟配置。 */
void DelayUs(unsigned char Time)
{
	unsigned char a;
	for(a=0;a<Time;a++)
	{
		NOP();
	}
}

/* 软件毫秒级延时，参数范围为0～255 ms。 */
void DelayMs(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<5;b++)
		{
		 	DelayUs(197);
		}
	}
}

/* 同步设置全部五组灯：State为1时点亮，为0时熄灭。 */
void LED_GROUPS_SET(unsigned char State)
{
	LedGroup1 = State;
	LedGroup2 = State;
	LedGroup3 = State;
	LedGroup4 = State;
	LedGroup5 = State;
}

/* 主程序：四次50 ms短闪，然后长灭500 ms并循环。 */
void main(void)
{
	POWER_INITIAL();

	while(1)
	{
		/* 第一次短闪。 */
		LED_GROUPS_SET(1);
		DelayMs(50);
		LED_GROUPS_SET(0);
		DelayMs(50);

		/* 第二次短闪。 */
		LED_GROUPS_SET(1);
		DelayMs(50);
		LED_GROUPS_SET(0);
		DelayMs(50);

		/* 第三次短闪。 */
		LED_GROUPS_SET(1);
		DelayMs(50);
		LED_GROUPS_SET(0);
		DelayMs(50);

		/* 第四次短闪。 */
		LED_GROUPS_SET(1);
		DelayMs(50);
		LED_GROUPS_SET(0);
		DelayMs(50);

		/* DelayMs参数为unsigned char，因此500 ms拆成两次250 ms。 */
		DelayMs(250);
		DelayMs(250);
	}
}



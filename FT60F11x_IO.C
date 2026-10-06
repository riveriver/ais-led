// Project:  FT60F11x_IO.prj
// Device:   FT60F11X
// Memory:   PROM=1Kx14, SRAM=64Byte, EEPROM=256Byte
// Description: ��DemoPortIn���ջ��߸ߵ�ƽʱ��DemoPortOut���?0Hzռ�ձ�50%�Ĳ��Σ���DemoPortIn�ӵ�ʱ��DemoPortOut����ߵ��?

// RELEASE HISTORY
// VERSION DATE     DESCRIPTION
// 1.1     24-2-21   �޸��ļ�ͷ

//*********************************************************
#include "SYSCFG.h"
//***********************�궨��****************************
#define  DemoPortOut	PA6   
#define  DemoPortIn		PC3
/*-------------------------------------------------
 * ��������POWER_INITIAL
 * ���ܣ�  �ϵ�ϵͳ��ʼ��
 * ���룺  ��
 * �����? ��
 --------------------------------------------------*/	
void POWER_INITIAL (void) 
{ 
	OSCCON = 0B01110001;	//IRCF=111=16MHz/2T=8MHz,0.125us
	INTCON = 0;  			//�ݽ�ֹ�����ж�
	PORTA = 0B00000000;		
	TRISA = 0B00000000;		//PA6-OUT
	PORTC = 0B00000000; 	
	TRISC = 0B00001000;		//PC3-IN
								
	WPUA = 0B00000000;    	//PA6 output pull-up disabled
	WPUC = 0B00001000;    	//PC3 input pull-up enabled
                            
	OPTION = 0B00001000;	//Bit3=1,WDT MODE,PS=000=WDT RATE 1:1                             
    PSRCA = 0B11111111;    	//Դ�����������?
    PSRCC = 0B11111111; 
    PSINKA = 0B11111111;    //������������
    PSINKC = 0B11111111;
                      
    MSCON = 0B00110000;		   	
	//Bit5:	PSRCAH4��PSRCA[4]��ͬ����Դ������00��4mA; 11: 33mA; 01��10:8mA
	//Bit4:	PSRCAH3��PSRCA[3]��ͬ����Դ������00��4mA; 11: 33mA; 01��10:8mA
	//Bit3:	UCFG1<1:0>Ϊ01ʱ��λ�����塣0����ֹLVR��	 1����LVR
	//Bit2:	��ʱ�Ӳ��������ڵ�ƽ��ģʽ��0���ر�ƽ��ģʽ��1����ƽ��ģʽ
	//Bit1:	0���رտ�ʱ�Ӳ��������ڣ�1���򿪿�ʱ�Ӳ���������
	//Bit0:	0��˯��ʱֹͣ������1�� ˯��ʱ���ֹ�����
    //		��T2ʱ�Ӳ���ѡ��ָ��ʱ�ӵ�ʱ
}
/*-------------------------------------------------
 * ��������interrupt ISR
 * ���ܣ�  �жϴ���
 * ���룺  ��
 * �����? ��
 --------------------------------------------------*/
void interrupt ISR(void)
{
}
/*----------------------------------------------------
 * �������ƣ�DelayUs
 * ���ܣ�    ����ʱ���� --16M-2T--��ſ�?%����.
 * ���������Time��ʱʱ�䳤�� ��ʱʱ��Time Us
 * ���ز������� 
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
 * �������ƣ�DelayMs
 * ���ܣ�    ����ʱ����
 * ���������Time��ʱʱ�䳤�� ��ʱʱ��Time ms
 * ���ز������� 
 ----------------------------------------------------*/
void DelayMs(unsigned char Time)
{
	unsigned char a,b;
	for(a=0;a<Time;a++)
	{
		for(b=0;b<5;b++)
		{
		 	DelayUs(197); 	//��1%
		}
	}
}
/*-------------------------------------------------
 *  ������: main 
 * ���ܣ�   ������
 *  ���룺  ��
 *  �����? ��
 --------------------------------------------------*/
void main(void)
{
	POWER_INITIAL();			//ϵͳ��ʼ��
    
	while(1)
	{
		/* First flash. */
		DemoPortOut = 1;
		DelayMs(80);
		DemoPortOut = 0;
		DelayMs(80);

		/* Second flash. */
		DemoPortOut = 1;
		DelayMs(80);
		DemoPortOut = 0;

		/* DelayMs takes an unsigned char, so split 600 ms into 3 calls. */
		DelayMs(200);
		DelayMs(200);
		DelayMs(200);
	}
}




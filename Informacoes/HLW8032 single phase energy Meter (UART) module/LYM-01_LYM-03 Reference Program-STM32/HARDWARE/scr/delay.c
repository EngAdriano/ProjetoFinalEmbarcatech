/*-------------------------------------------------*/
/*                 美岩电子制作                    */
/*-------------------------------------------------*/
/*            实现延时功能的源文件                 */
/*-------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "delay.h"      //包含需要的头文件
static unsigned char  Fac_Us=0;//Us延时倍乘数
static unsigned int Fac_Ms=0;//Ms延时倍乘数
/*-------------------------------------------------*/
/*函数名：初始化延时函数                           */
/*参  数：Sysclk，主时钟                           */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Delay_Init(unsigned char Sysclk)	 
{
    SysTick->CTRL&=0xfffffffb;//清空Bit,选择外部时钟，HCLK/8
	Fac_Us=Sysclk/8;
    Fac_Ms=(u16)Fac_Us*1000;
}			

/*-------------------------------------------------*/
/*函数名：延迟微秒函数                             */
/*参  数：us：延时多少微秒                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Delay_Us(unsigned int us)
{		
    unsigned long Temp;       	 
    SysTick->LOAD=us*Fac_Us;//时间加载      		 
    SysTick->VAL=0x00;//清空计数器
    SysTick->CTRL=0x01;//开始倒数 	 
    do
    {
        Temp=SysTick->CTRL;
    }
    while(Temp&0x01&&!(Temp&(1<<16)));//等待时间到达   
    SysTick->CTRL=0x00;//关闭计数器
    SysTick->VAL =0X00;//清空计数器	  
}

/*-------------------------------------------------*/
/*函数名：延迟微秒函数                             */
/*参  数：ms：延时多少毫秒                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Delay_Ms(unsigned int ms)
{	 		  	  
    unsigned long Temp;  	   
    SysTick->LOAD=(unsigned long)ms*Fac_Ms;//时间加载(SysTick->LOAD为24Bit)
    SysTick->VAL =0x00;//清空计数器
    SysTick->CTRL=0x01;//开始倒数
    do
    {
			Temp=SysTick->CTRL;
    }
    while(Temp&0x01&&!(Temp&(1<<16)));//等待时间到达
    SysTick->CTRL=0x00;//关闭计数器
    SysTick->VAL =0X00;//清空计数器   
} 

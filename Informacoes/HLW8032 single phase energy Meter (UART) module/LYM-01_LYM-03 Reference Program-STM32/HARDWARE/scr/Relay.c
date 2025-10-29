

#include "stm32f10x.h"  //包含需要的头文件
#include "Relay.h"        //包含需要的头文件

/*-------------------------------------------------*/
/*函数名：初始化LED函数                            */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void Relay_Init(void)
{    	 
	GPIO_InitTypeDef GPIO_InitStructure;                     //定义一个设置GPIO的变量
		
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);                                //使能GPIOA端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;                                             //准备设置PA5
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                                     //IO速率50Mhz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   		                              //推免输出方式
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		                              //设置PA5

	Relay_OFF;
}


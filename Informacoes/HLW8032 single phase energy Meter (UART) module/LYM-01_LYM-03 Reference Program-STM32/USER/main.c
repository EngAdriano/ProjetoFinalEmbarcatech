/*-----------------------------------------------------*/
/*                                                     */
/*           程序main函数，入口函数源文件              */
/*                                                     */
/*-----------------------------------------------------*/

#include "stm32f10x.h"  //包含需要的头文件
#include "delay.h"      //包含需要的头文件
#include "usart1.h"     //包含需要的头文件
#include "usart3.h"     //包含需要的头文件
#include "Relay.h"        //包含需要的头文件
#include "key.h"        //包含需要的头文件


int main(void) 
{	
	Delay_Init(72);                   //延时功能初始化              
	Usart1_Init(9600);              //串口1功能初始化，波特率9600
	Usart3_Init(4800);              //串口3功能初始化，波特率4800
	Relay_Init();	                    //LED初始化
	KEY_Init();                     //按键初始化
	
	while(1)                        //主循环
	{		
		USART_Cmd(USART3, ENABLE);  //使能串口3 ,读取电参数
		Delay_Ms(200);
		u1_printf("当前电压=%.1fV,当前电流=%.3fA,实时功率=%.1fW,功率因数=%.3f,累计电量=%.1fkW.h\r\n"\
		,DisplayTable[0],DisplayTable[1],DisplayTable[2],DisplayTable[3],DisplayTable[4]);
		Delay_Ms(1500); //两秒左右读取一次
	}
}


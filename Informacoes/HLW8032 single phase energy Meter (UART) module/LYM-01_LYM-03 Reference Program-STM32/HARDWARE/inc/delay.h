/*-------------------------------------------------*/
/*            超纬电子STM32系列开发板              */
/*-------------------------------------------------*/
/*                                                 */
/*          使用SysTick实现延时功能的头文件        */
/*                                                 */
/*-------------------------------------------------*/

#ifndef __DELAY_H
#define __DELAY_H 

void Delay_Init(unsigned char Sysclk);
void Delay_Us(unsigned int);   //延时毫秒函数
void Delay_Ms(unsigned int);   //延时微秒函数

#endif






























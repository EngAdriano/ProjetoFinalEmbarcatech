

#ifndef __RELAY_H
#define __RELAY_H

#define Relay_OUT(x)   GPIO_WriteBit(GPIOA, GPIO_Pin_5,  (BitAction)x)  //设置PA5  的电平，可以点亮熄灭LED1

#define Relay_IN_STA   GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_5)  //PA5  控制继电器,读取电平状态，判断继电器是吸合还是弹起

#define Relay_ON       GPIO_ResetBits(GPIOA, GPIO_Pin_5)         //共阳极，拉低PA5电平，吸合继电器
#define Relay_OFF      GPIO_SetBits(GPIOA, GPIO_Pin_5)           //共阳极，拉高PA5电平，弹起继电器


void Relay_Init(void);               //初始化	


#endif

/*-------------------------------------------------*/
/*                 美岩电子制作                    */
/*-------------------------------------------------*/
/*           实现LCD12864串行显示功能的头文件      */
/*-------------------------------------------------*/

#ifndef __LCD12864S_H
#define __LCD12864S_H

#define LCD_RST_L      GPIO_ResetBits(GPIOB, GPIO_Pin_12)                //拉低PB12电平，RST置低电平
#define LCD_RST_H      GPIO_SetBits(GPIOB, GPIO_Pin_12)                  //拉高PB12电平，RST置高电平

#define LCD_SCLK_L     GPIO_ResetBits(GPIOB, GPIO_Pin_13)                //拉低PB13电平，SCLK置低电平
#define LCD_SCLK_H     GPIO_SetBits(GPIOB, GPIO_Pin_13)                  //拉高PB13电平，SCLK置高电平

#define LCD_SID_L      GPIO_ResetBits(GPIOB, GPIO_Pin_14)                //拉低PB14电平，SID置低电平
#define LCD_SID_H      GPIO_SetBits(GPIOB, GPIO_Pin_14)                  //拉高PB14电平，SID置高电平

#define LCD_CS_L       GPIO_ResetBits(GPIOB, GPIO_Pin_15)                //拉低PB15电平，CS置低电平
#define LCD_CS_H       GPIO_SetBits(GPIOB, GPIO_Pin_15)                  //拉高PB15电平，CS置高电平

void LCD12864_Init(void);                                                //初始化LCD12864函数	
void LCD_Write_COM(unsigned char COMdode);                               //LCD写指令函数	
void LCD_Write_DAT(unsigned char DATdode);                               //LCD写数据函数
void LCD_Set_Position(unsigned char x, unsigned char y);                 //设置LCD显示坐标
void LCD_Display_String(unsigned char x,unsigned char y,unsigned char *DisplayString);
                                                                         //显示字符串
void LCD_Display_Num(unsigned char x,unsigned char y,unsigned char lenth,unsigned int DisplayNum);
                                                                         //显示一个数字
void LCD_Display_Num_point(unsigned char x,unsigned char y,unsigned char point,unsigned int DisplayNum);
                                                                         //显示一个带小数点的数字

#endif

/*-------------------------------------------------*/
/*                 美岩电子制作                    */
/*-------------------------------------------------*/
/*           实现继LCD串行显示功能的源文件         */
/*-------------------------------------------------*/

#include "stm32f10x.h"   //包含需要的头文件
#include "LCD12864S.h"   //包含需要的头文件
#include "delay.h"       //包含需要的头文件
/*-------------------------------------------------*/
/*函数名：初始化LCD12864函数                       */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LCD12864_Init(void)
{			
	GPIO_InitTypeDef  GPIO_InitStructure;                      //定义一个设置IO的变量

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );    //使能GPIOB时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;   
                                                               //准备设置RST-PB12,SCLK-PB13,SID-PB14,CS-PB15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //推免输出方式  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;          //IO速率50M
	GPIO_Init(GPIOB, &GPIO_InitStructure);                     //设置PB6 PB7
   
    LCD_RST_L; Delay_Ms(1);                                   
    LCD_RST_H; Delay_Ms(100);                                  //复位LCD，等待LCD自检，大于40ms
    LCD_Write_COM(0x30); Delay_Us(150);                        //功能设置，延时大于100uS
    LCD_Write_COM(0x30); Delay_Us(40);                         //功能设置，延时大于37uS
    LCD_Write_COM(0x0c); Delay_Us(150);                        //开显示，延时大于100uS
    LCD_Write_COM(0x01); Delay_Ms(15);                         //清屏，延时大于10ms
    LCD_Write_COM(0x06);                                       //进入设定点，完成初始化
}

/*-------------------------------------------------*/
/*函数名：LCD12864写字节函数                       */
/*参  数：Byte，需要写入的字节                     */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LCD12864_SendByte(unsigned char Byte)
{
    unsigned char i;
    for(i=0; i<8; i++){
        if((Byte<<i)&0x80){
            LCD_SID_H;
        }
        else{
            LCD_SID_L;
        }
        LCD_SCLK_L;	
        LCD_SCLK_H;	
    }
}

/*-------------------------------------------------*/
/*函数名：LCD12864写指令函数                       */
/*参  数：COMdode，需要写入的指令                  */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LCD_Write_COM(unsigned char COMdode)
{
    LCD_CS_H;			                 //CS置高电平，准备发送数据
    Delay_Us(100);
    LCD12864_SendByte(0xF8);             //0xF8为写指令
    LCD12864_SendByte(COMdode&0xF0);     //写高4位
    LCD12864_SendByte((COMdode<<4)&0xF0);//写低4位
    Delay_Us(100);
}
/*-------------------------------------------------*/
/*函数名：LCD12864写数据函数                       */
/*参  数：DATdode，需要写入的数据                  */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LCD_Write_DAT(unsigned char DATdode)
{
    LCD_CS_H;			                 //CS置高电平，准备发送数据
    Delay_Us(100);
    LCD12864_SendByte(0xFA);             //0xFA为写数据
    LCD12864_SendByte(DATdode&0xF0);     //写高4位
    LCD12864_SendByte((DATdode<<4)&0xF0);//写低4位
    Delay_Us(100);
}

/*-------------------------------------------------*/
/*函数名：计算m^n                                  */
/*参  数：m:为底数,n:为幂                          */
/*返回值：计算结果                                 */
/*-------------------------------------------------*/
unsigned int Compute_Power(unsigned char m,unsigned char n)
{
	unsigned int result=1;
	while(n--)
	{
	  result*=m;
	}
	return result;
}

/*-------------------------------------------------*/
/*函数名：设置LCD显示坐标                          */
/*参  数：x:行,1-4，y:列，0-7                      */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LCD_Set_Position(unsigned char x, unsigned char y)
{
    switch(x){
        case 1:   LCD_Write_COM(0x80 + y); break;	
		case 2:   LCD_Write_COM(0x90 + y); break;	
        case 3:   LCD_Write_COM(0x88 + y); break;	
        case 4:   LCD_Write_COM(0x98 + y); break;	        
        default:  LCD_Write_COM(0x80 + y); break;	 
    } 
}

/*-------------------------------------------------*/
/*函数名：LCD显示一组字符串                        */
/*参  数：x:行,1-4，y:列，0-7                      */
/*        DisplayString:要显示的字符串             */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LCD_Display_String(unsigned char x,unsigned char y,unsigned char *DisplayString)
{ 
    LCD_Set_Position(x,y);
 	while(*DisplayString>0)
   { 
      LCD_Write_DAT(*DisplayString);
      DisplayString++;
   }
}

/*-------------------------------------------------*/
/*函数名：LCD显示一个数字                          */
/*参  数：x:行,1-4，y:列,0-7, lenth:显示数字的位数 */
/*        DisplayNum:要显示的数字,最大65535        */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LCD_Display_Num(unsigned char x,unsigned char y,unsigned char lenth,unsigned int DisplayNum)
{ 
    unsigned char t,temp;
    LCD_Set_Position(x,y);
 	for(t=0;t<lenth;t++){
        temp=(DisplayNum/Compute_Power(10,lenth-t-1))%10;
        LCD_Write_DAT(temp+'0');
    }
}

/*-------------------------------------------------*/
/*函数名：LCD显示一个带小数点的数字                */
/*参  数：x:行,1-4，y:列,0-7, point:小数点的位数   */
/*        DisplayNum:要显示的数字,最大9999         */
/*返回值：无                                       */
/*-------------------------------------------------*/
void LCD_Display_Num_point(unsigned char x,unsigned char y,unsigned char point,unsigned int DisplayNum)
{ 
    unsigned char t,temp;
    LCD_Set_Position(x,y);
		if(point==0){
			for(t=0;t<4;t++){
				temp=(DisplayNum/Compute_Power(10,3-t))%10;
				LCD_Write_DAT(temp+'0');
			}
			if(DisplayNum/1000==0)
				LCD_Write_DAT(' ');
		}
    if(point==1){
        for(t=0;t<2;t++){
					temp=(DisplayNum/Compute_Power(10,3-t))%10;
					LCD_Write_DAT(temp+'0');
				}
			LCD_Write_DAT('.');
			LCD_Write_DAT(DisplayNum%10+'0');
    }
    if(point==2){
			LCD_Write_DAT(DisplayNum/100+'0');
			LCD_Write_DAT('.');
			LCD_Write_DAT(DisplayNum/10%10+'0');
			LCD_Write_DAT(DisplayNum%10+'0');
    }
}

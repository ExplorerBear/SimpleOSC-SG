#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include "i2c.h"
#include "OLED_Data.h"
#include <stdlib.h>

/** 项目配置：需要在项目选项中 C/C++ -> Misc Control 中添加 "--no-multibyte-chars"，以适配中文。
	* 					显存需要 1Kb 的heap空间，以及发送数据缓冲区最大需要128Bytes的Heap空间，因此Heap至少需要配置为0x480
	*/

/*参数宏定义*********************/

#define IIC hi2c2	//Used IIC interface
#define IIC_Addr 0x78	//Driver Addr
#define IIC_Mode_Blocking	//IIC传输方式，阻塞/中断/DMA，按下面的三个选项
/** IIC Mode Type****

	IIC_Mode_Blocking //阻塞方式
	IIC_Mode_IT				//中断方式
	IIC_Mode_DMA			//DMA方式
*/


/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define OLED_8X16				8
#define OLED_6X8				6

/*IsFilled参数数值*/
#define OLED_UNFILLED			0
#define OLED_FILLED				1

/* 全局变量 */
extern uint8_t *SendByte_Addr;//一个Page的数据指针
extern uint8_t *OLED_DisplayBuf;//显存地址（如果需要则声明）

/*********************参数宏定义*/

/***********函数列表**********/

void OLED_Transmit_Datas(void);//IT/DMA模式下使用的传输函数
void OLED_WriteData(uint8_t *Data, uint8_t Count);

/* Init and OLED Func 初始化和功能函数 */
void OLED_Init(void);//初始化 
void OLED_DeInit(void);
void OLED_SetCursor(uint8_t Page, uint8_t X);// 设置光标
void OLED_Update(void);//更新显存数组到屏幕
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);//更新区域显存数组到屏幕
void OLED_Clear(void);//显存数组清零
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);//部分显存数组清零
void OLED_Reverse(void);//显存数组取反
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
void OLED_Test(void);//屏幕测试

/* String 字符 */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);//显示一个字符
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);//显示字符串
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);//显示无符号十进制数字
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);//显示有符号数字
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);//显示十六进制数字
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);//显示二进制数字
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);//显示浮点数
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);//显示图像
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);//在OLED以printf打印格式化字符串

/* Draw 绘图 */
void OLED_DrawPoint(int16_t X, int16_t Y);//画点
uint8_t OLED_GetPoint(int16_t X, int16_t Y);//读取点
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);//绘制直线
void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);//绘制矩形
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);//绘制三角
void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled);//绘制圆
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);//绘制椭圆
void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);//绘制圆弧

/** 中断/DMA 模式下需要配置的中断回调函数 
	*
	* 说明：此段程序只需将其复制到HAL_I2C_MasterTxCpltCallback()函数中，即可			
	*/
/*
	if(hi2c == &IIC)
	{
		free(SendByte_Addr);//单次传输一行数据完毕，释放此内存区域防止内存溢出
		OLED_Transmit_Datas();
	}
*/


#endif

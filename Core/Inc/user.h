#ifndef __USER_H
#define __USER_H

#include "main.h"
#include "stm32h7xx_hal_uart.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_def.h"

#define	SinWave	0
#define	PWMWave	1 
#define	TriangleWave 2
#define Volt_Deflut 3.3

#define DataNum 100 // DAC 缓冲区大小
#define Rx_Buff_Size 20 // 串口接收缓冲区大小


/* DAC输出波形配置结构体 */
typedef struct{
	uint16_t Freq;
	uint8_t Duty;
	uint8_t Wave;
	float V_H;
	
}DAC_PWM_Config;


//void ADC_DataCount(uint16_t *DataBuff, uint16_t *Value_Max, uint16_t *Value_Min);//已做到 ADC_Process() 中，不再使用
uint32_t TIM_SUM(uint32_t *arr,uint8_t Number);
void ADC_Process(uint16_t *DataBuff);
void TIM2_Process(uint32_t *TimCC_Value, uint8_t Data_Num);
void DAC_PWM_Process(void);
void UART_Process(void);// 外设状态处理函数
void SinWave_Generate(uint16_t *Databuf);
void PWMWave_Generate(uint16_t *Databuf);
void TrgWave_Generate(uint16_t *Databuf);//波形生成
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *huart, uint8_t *pData,uint16_t Size);



#endif

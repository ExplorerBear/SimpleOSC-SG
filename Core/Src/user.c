
#include "user.h"
#include "OLED.h"
#include <stdio.h>
#include "string.h"

#define PI 3.1415926535

/* 全局变量 */
extern uint8_t ADC_state;//ADC状态标志
extern uint8_t Tim_State;//定时器2状态标志
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern DAC_HandleTypeDef hdac1;
extern UART_HandleTypeDef huart1;//外设操作句柄

//extern uint16_t SinWave_Data[DataNum];
__attribute__((section(".RAM_D1"))) uint16_t SinWave_Data[DataNum];
__attribute__((section(".RAM_D1"))) uint16_t PWMWave_Data[DataNum];
__attribute__((section(".RAM_D1"))) uint16_t TrgWave_Data[DataNum];		//	DAC 数据缓冲区
uint8_t USRT_Rx_Buff[Rx_Buff_Size];//串口接收缓冲区
uint8_t uart_state = 0;//串口状态标志位

/* DAC 配置结构体 */
DAC_PWM_Config Wave_Config={
	1000,50,SinWave,Volt_Deflut
};

/* 函数声明 */

uint8_t Check_str(uint8_t *str1,const uint8_t *str2,uint8_t hand,uint8_t Number);//字符比较函数
void Freq_Set(void);
void Duty_Set(void);

/* AT 指令集 */
const uint8_t AT[]="AT+";
const uint8_t SIN[] = "Sin+";
const uint8_t Freq[] = "Freq=";
const uint8_t PWM[] = "PWM+";
const uint8_t Duty[] = "Duty=";
const uint8_t Trg[] = "Trg+";

	
/** ADC数据处理函数包含取最大最小值
	*	以及将采集的数据写入至屏幕显存
	*	已做到ADC_Process()中，不再使用
	*/

/*
void ADC_DataCount(uint16_t *DataBuff, uint16_t *Value_Max, uint16_t *Value_Min)
{
	uint8_t Y,i;
	uint16_t Max,Min;
	Max = 0;
	Min = 65535;
	for(i = 0; i<128; i++)
	{
		Max = ((*(DataBuff+i))>Max)? *(DataBuff+i): Max;
		Min = ((*(DataBuff+i))<Min)? *(DataBuff+i): Min;
		Y = (*(DataBuff+(i)))*(40.000000/65535);
		OLED_DrawPoint(i,64-8-Y);
		//printf("%d\n",Y);
	}
	*Value_Max = Max;
	*Value_Min = Min;
}
*/

/** 定时器求和函数
	* 参	数：arr 数据数组指针
	* 参	数：Number 数据的数量
	*/
uint32_t TIM_SUM(uint32_t *arr,uint8_t Number)
{
	uint32_t sum=0;
	for(uint8_t i=0; i<Number; i++)
	{
		sum += *arr;
	}
	return sum;
}

/** ADC 运行语句块
	*/
void ADC_Process(uint16_t *DataBuff)
{
	uint8_t Y,i;
	uint16_t Max,Min,Value_Max,Value_Min;
	float V_Max,V_Min;
	
	Max = 0;
	Min = 65535;
	
	if(ADC_state == 1)
		{
			HAL_TIM_Base_Stop(&htim6);
			HAL_ADC_Stop_DMA(&hadc1);//停止转换
			OLED_Clear();
			//ADC_DataCount(ADC_Values,&num_max,&num_min);
			
				
			for(i = 0; i<128; i++)//计算显示的点的坐标并加载到显存中
			{
				Max = ((*(DataBuff+i))>Max)? *(DataBuff+i): Max;
				Min = ((*(DataBuff+i))<Min)? *(DataBuff+i): Min;//计算最大最小的ADC值
				Y = (*(DataBuff+(i)))*(40.000000/65535);
				OLED_DrawPoint(i,64-8-Y);
				//printf("%d\n",Y);
			}
			Value_Max = Max;
			Value_Min = Min;
				
			//printf("one end\n");
			ADC_state = 0;
		}
		if(ADC_state == 0)
		{
			HAL_ADC_Start_DMA(&hadc1,(uint32_t *)DataBuff,128);	
			HAL_TIM_Base_Start(&htim6);
		}
		
		V_Max = (3.3/65535)*Value_Max;
		V_Min = (3.3/65535)*Value_Min;//转换为电压值输出到显存
		OLED_ShowFloatNum(0,57,V_Max,1,2,OLED_6X8);
		OLED_ShowFloatNum(48,57,V_Min,1,2,OLED_6X8);
}

/** TIM2 处理函数
	*/
void TIM2_Process(uint32_t *TimCC_Value, uint8_t Data_Num)
{
	uint32_t TimCC_Value_T_T[Data_Num-1];
	uint32_t TimCC_Avg;
	uint16_t Tim6_Arr_Value,Freq;
	
	if(Tim_State == 1)
		{
			for(uint8_t i = 0; i<sizeof(TimCC_Value_T_T);i++)
			{
				TimCC_Value_T_T[i] = TimCC_Value[i+1] - TimCC_Value[i];
			}
			TimCC_Avg = TIM_SUM(TimCC_Value_T_T,sizeof(TimCC_Value_T_T))/sizeof(TimCC_Value_T_T);//计算定时器输入捕获的计数值差的平均值
			Freq = 1/(TimCC_Avg*3.34/1000000);//定时器CNT一个计数周期为3.34 um，计算频率
			
			 
			Tim6_Arr_Value = 3000000/(Freq*25);//设置ADC采样为频率的25倍
			if(Tim6_Arr_Value < 25)
			{
				TIM6->ARR = 25;
			}
			else
			{
				if(Tim6_Arr_Value > 30000)
				{
					TIM6->ARR = 30000;
				}
				else TIM6->ARR = Tim6_Arr_Value;
				
			}
			Tim_State = 0;
		}
		
		if(Tim_State == 0)
		{
			HAL_TIM_IC_Start_DMA(&htim2,TIM_CHANNEL_3,TimCC_Value,Data_Num);
		}
		
		OLED_ShowNum(0,0,Freq,6,OLED_8X16);
}

/** DAC&PWM 处理函数
	*/
void DAC_PWM_Process(void)
{
	
	TIM4->ARR= 100000/Wave_Config.Freq;
	
	if(Wave_Config.Wave == SinWave)
	{
		HAL_DAC_Start_DMA(&hdac1,DAC_CHANNEL_1,(uint32_t *)SinWave_Data,DataNum,DAC_ALIGN_12B_R);
		HAL_TIM_Base_Start(&htim4);
	}
	
	if(Wave_Config.Wave == PWMWave)
	{
		PWMWave_Generate(PWMWave_Data);
		/* 使用定时器比较输出 */
		//TIM4->CCR1 = TIM4->ARR/Wave_Config.Dute;
		//TIM4->CCR1 = 400;
		//HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
		HAL_DAC_Start_DMA(&hdac1,DAC_CHANNEL_1,(uint32_t *)PWMWave_Data,DataNum,DAC_ALIGN_12B_R);
		HAL_TIM_Base_Start(&htim4);
	}
	
	if(Wave_Config.Wave == TriangleWave)
	{
		HAL_DAC_Start_DMA(&hdac1,DAC_CHANNEL_1,(uint32_t *)TrgWave_Data,DataNum,DAC_ALIGN_12B_R);
		HAL_TIM_Base_Start(&htim4);
	}
}

/** sin数据生成函数
	* 参	数：sin数据缓冲区地址
	*/
void SinWave_Generate(uint16_t *Databuf)
{
	for(uint16_t n=0; n<DataNum;n++)
	{
		*(Databuf+n) = (uint16_t)(2047+2047*sin(2.000000*PI*n/DataNum/1.0000000));
	}
	//SCB_CleanDCache_by_Addr((uint32_t *)Databuf,DataNum);
}

/** PWM DAC 数据生成
	* 参	数：PWM 数据缓冲区地址
	*/
void PWMWave_Generate(uint16_t *Databuf)
{
	uint8_t i;
	for(i=0;i<Wave_Config.Duty;i++)
	{
		*(Databuf+i) = 4095;
	}
	for(;i<100;i++)
	{
		*(Databuf+i) = 0;
	}
}

/** 三角波数据生成
	* 参	数：三角波数据缓冲区地址
	*/
void TrgWave_Generate(uint16_t *Databuf)
{
	uint8_t i;
	
	*Databuf = 0;
	for(i=0;i<51;i++)
	{
		*(Databuf+i+1) = (uint16_t)(*(Databuf+i) + 81.9);
	}
	for(;i<100;i++)
	{
		*(Databuf+i) = (uint16_t)(*(Databuf+i-1) - 81.9);
	}
}

/** 串口接收处理函数
	* 串口接收有三个状态：0：开始准备接收，1：串口接收中，2：串口接收完成
	*/
void UART_Process(void)
{
	if(uart_state == 2)
	{
		if(Check_str(USRT_Rx_Buff,AT,0,3) == 0)
    {
        if(Check_str(USRT_Rx_Buff,SIN,3,4) == 0)
        {
					Wave_Config.Wave = SinWave;
					Freq_Set();
        }else
				{
					if(Check_str(USRT_Rx_Buff,PWM,3,4) == 0)
					{
						Wave_Config.Wave = PWMWave;
						Freq_Set();
						Duty_Set();
					}else{
					if(Check_str(USRT_Rx_Buff,Trg,3,4) == 0)
					{
						Wave_Config.Wave = TriangleWave;
						Freq_Set();
					}
					}
				}
				HAL_DAC_Stop_DMA(&hdac1,DAC_CHANNEL_1);
				HAL_TIM_Base_Stop(&htim4);
				DAC_PWM_Process();
				
				printf("+OK\n");
    }
		
		uart_state = 0;
	}
	
	if(uart_state == 0)
	{
		uart_state=1;
		HAL_UART_DMAStop(&huart1);//停止DMA传输，重置缓冲区指针
		memset(USRT_Rx_Buff,0,20);
		__HAL_UART_CLEAR_IDLEFLAG(&huart1);//清除空闲中断标志位
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1,USRT_Rx_Buff,Rx_Buff_Size);
	}
}

/** 字符串比较函数 ——逐字比较法
	* 参  数：str1	需要被比较的字符串指针
	* 参	数：str2	比较的字符串指针
	* 参	数：hand	被比较的起始字符索引
	* 参	数：Number	比较的字符数
	* 返回值：0表示比较结果相同，1表示比较结果不同
	*/
uint8_t Check_str(uint8_t *str1,const uint8_t *str2,uint8_t hand,uint8_t Number)
{
    for (uint8_t i =0;i<Number; i++)
    {
        if ((*(str1+hand+i)) != (*(str2+i))) return 1;
    }
    return 0;
}

/** 读取并设置频率
	*/
void Freq_Set(void)
{
	uint16_t NUM=0;
	uint8_t n=0,b=0;
	uint8_t rx_Data[6];
	if(Check_str(USRT_Rx_Buff,Freq,7,5) == 0)
	{
			while((*(USRT_Rx_Buff+12+n) != 0))
			{
					rx_Data[n] = *(USRT_Rx_Buff+12+n);
					n++;
			}
			while(n!=0)
			{
					NUM += (rx_Data[b]-48)*pow(10,n-1);
					n--;
					b++;
			}
			Wave_Config.Freq = NUM;
			//TIM4->ARR = 100000/NUM;
	}
	return;
}

/** 读取并设置占空比
	*/
void Duty_Set(void)
{
	uint16_t NUM=0;
	uint8_t n=0,b=0;
	uint8_t rx_Data[6];
	if(Check_str(USRT_Rx_Buff,Duty,7,5) == 0)
	{
			while((*(USRT_Rx_Buff+12+n) != '\0'))
			{
					rx_Data[n] = *(USRT_Rx_Buff+12+n);
					n++;
			}
			while(n!=0)
			{
					NUM += (rx_Data[b]-48)*pow(10,n-1);
					n--;
					b++;
			}
			Wave_Config.Duty = NUM;
			//TIM4->ARR = 100000/NUM;
	}
	return;
}

/** 串口接收库函数补充
	*/ 
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *huart, uint8_t *pData,uint16_t Size)
{
	HAL_StatusTypeDef status;
	/* 开启空闲中断 */
  __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
	
	 /* 启动 DMA 接收 */
	status = HAL_UART_Receive_DMA(huart, pData, Size);
	//if (status != HAL_OK)
	return status;

}



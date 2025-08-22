#include "OLED_UI_Driver.h"

/*
【文件说明】：[硬件抽象层]
此文件包含按键与编码器的驱动程序，如果需要移植此项目，请根据实际情况修改相关代码。
当你确保oled屏幕能够正常点亮，并且能够正确地运行基础功能时（如显示字符串等），就可以开始移植
有关按键与编码器等的驱动程序了。
*/


/**
 * @brief 定时器中断服务函数的初始化函数，用于产生20ms的定时器中断
 * @param 无
 * @return 无
 */
void Timer_Init(void)
{
    /* 使能TIM3时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);            // 使能TIM3时钟
    
    /* 配置定时器时钟源为内部时钟 */
    TIM_InternalClockConfig(TIM3);        // 配置TIM3使用内部时钟
    
    /* 配置定时器基准参数 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;                // 定义定时器基准配置结构体
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;        // 时钟分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;    // 向上计数模式
    TIM_TimeBaseInitStructure.TIM_Period = 200 - 1;                    // 自动重装载寄存器ARR，定时周期为20ms
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;                // 预分频器，定时器时钟频率为10kHz
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;                // 重复计数器不使用
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);                // 初始化TIM3基准配置
    
    /* 清除定时器更新中断标志 */
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);                            // 清除更新中断标志位
    
    /* 使能定时器更新中断 */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                        // 使能TIM3的更新中断
    
    /* 配置中断优先级 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                    // 设置NVIC中断优先级分组为2
    
    /* 配置NVIC中断 */
    NVIC_InitTypeDef NVIC_InitStructure;                            // 定义NVIC初始化结构体
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;                    // 设置中断源为TIM3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                    // 使能中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;        // 预优先级为2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;                // 子优先级为1
    NVIC_Init(&NVIC_InitStructure);                                    // 初始化NVIC中断配置
    
    /* 使能定时器 */
    TIM_Cmd(TIM3, ENABLE);            // 使能定时器3
}

/**
 * @brief 按键初始化函数，用于初始化按键GPIO
 * @param 无
 * @note GPIO被初始化为上拉输入模式（虽然在我的开发板上已经加上了上拉电阻，但是以防万一）
 * @return 无
 */
void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}



/**
 * @brief 编码器初始化函数，将定时器1配置为编码器模式
 * @param 无
 * @return 无
 */
void Encoder_Init(void)
{
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;		//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;		//PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter = 0xF;
	TIM_ICInit(TIM1, &TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICFilter = 0xF;
	TIM_ICInit(TIM1, &TIM_ICInitStructure);
	
	TIM_EncoderInterfaceConfig(TIM1, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	
	TIM_Cmd(TIM1, ENABLE);
	
	TIM_SetCounter(TIM1, 0);
}


/**
 * @brief 编码器使能函数
 * @param 无
 * @return 无
 */
void Encoder_Enable(void)
{
	TIM_Cmd(TIM1, ENABLE);
}

/**
 * @brief 编码器失能函数
 * @param 无
 * @return 无
 */
void Encoder_Disable(void)
{
	TIM_Cmd(TIM1, DISABLE);
}

/**
 * @brief 获取编码器的增量计数值（四倍频解码）
 * 
 * @details 该函数通过读取定时器TIM1的计数值，对编码器信号进行四倍频解码处理。
 *          使用静态变量累积计数，并通过除法和取模运算去除多余的增量部分，
 *          确保返回精确的增量值。主要用于电机控制、位置检测等应用场景。
 * 
 * @note   函数内部会自动清零定时器计数值，确保下次读取的准确性
 * 
 * @return int16_t 返回解码后的编码器增量值
 */
int16_t Encoder_Get(void)
{
    // 静态变量，用于在函数调用间保存未被4整除的余数
    static int32_t encoderAccumulator = 0;
    
    // 读取当前定时器计数值
    int16_t temp = TIM_GetCounter(TIM1);
    
    // 清零定时器计数器，为下次读取做准备
    TIM_SetCounter(TIM1, 0);
    
    // 将当前读取值累加到累加器中
    encoderAccumulator += temp;
    
    // 计算四倍频解码后的增量值（去除未完成的部分）
    int16_t result = encoderAccumulator / 4;
    
    // 保存未被4整除的余数，保证精度
    encoderAccumulator %= 4;
    
    // 返回解码后的增量值
    return result;
}



/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
void Delay_us(uint32_t xus)
{
	SysTick->LOAD = 72 * xus;				//设置定时器重装值
	SysTick->VAL = 0x00;					//清空当前计数值
	SysTick->CTRL = 0x00000005;				//设置时钟源为HCLK，启动定时器
	while(!(SysTick->CTRL & 0x00010000));	//等待计数到0
	SysTick->CTRL = 0x00000004;				//关闭定时器
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 



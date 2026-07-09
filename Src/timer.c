#include "common.h"
#include "timer.h"
#include "stm32f10x_tim.h"
#include "misc.h"


volatile uint32_t timer2_cur_time_ms = 0; 
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 
    {
        timer2_cur_time_ms++;
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}

void Tim2_count_mode_up(void)
{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		TIM_TimeBaseInitTypeDef timer_init;
		timer_init.TIM_Prescaler         = 36000 - 1;
		timer_init.TIM_CounterMode       = TIM_CounterMode_Up;
		timer_init.TIM_Period            = 10 - 1; 
		timer_init.TIM_ClockDivision     = TIM_CKD_DIV1;
		timer_init.TIM_RepetitionCounter = 0;
		
		TIM_TimeBaseInit(TIM2, &timer_init);
		TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
		NVIC_InitTypeDef nvic_init;
		nvic_init.NVIC_IRQChannel = TIM2_IRQn;
		nvic_init.NVIC_IRQChannelPreemptionPriority = NVIC_TIM2_PRIORITY;
		nvic_init.NVIC_IRQChannelSubPriority = 0;
		nvic_init.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic_init);
			
		TIM_Cmd(TIM2, ENABLE);
}

volatile uint8_t tim3_10sec_flag = 0;
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) 
    {
        tim3_10sec_flag = 1;
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

void Tim3_init_10sec_timer(void)
{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		TIM_TimeBaseInitTypeDef timer_init;
		timer_init.TIM_Prescaler         = 40000 - 1;
		timer_init.TIM_CounterMode       = TIM_CounterMode_Up;
		timer_init.TIM_Period            = 18000 - 1; 
		timer_init.TIM_ClockDivision     = TIM_CKD_DIV1;
		timer_init.TIM_RepetitionCounter = 0;
		
		TIM_TimeBaseInit(TIM3, &timer_init);
		TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
		NVIC_InitTypeDef nvic_init;
		nvic_init.NVIC_IRQChannel = TIM3_IRQn;
		nvic_init.NVIC_IRQChannelPreemptionPriority = NVIC_TIM3_PRIORITY;
		nvic_init.NVIC_IRQChannelSubPriority = 0;
		nvic_init.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic_init);
			
		TIM_Cmd(TIM3, ENABLE);
}

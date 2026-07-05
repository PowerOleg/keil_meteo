#include "common.h"

volatile uint32_t delay_us = 0;
void SysTick_Handler(void)
{
	if (delay_us > 0)
		delay_us--;
}

void Delay_us(uint32_t delay_new)
{
	delay_us = delay_new;
	while(delay_us) {}
}


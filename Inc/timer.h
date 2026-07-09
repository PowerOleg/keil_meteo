#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"

#define TIMEOUT_END_200_MS 200

extern volatile uint32_t timer2_cur_time_ms; 
void Tim2_count_mode_up(void);

extern volatile uint8_t tim3_10sec_flag;
void Tim3_init_10sec_timer(void);

#endif

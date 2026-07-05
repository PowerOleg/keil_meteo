#ifndef __RTC_FUNCTIONS_H
#define __RTC_FUNCTIONS_H

#include "stm32f10x.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_rtc.h"

// (UnixTime = 00:00:00 01.01.1970 = JD0 = 2440588)
#define JULIAN_DATE_BASE 2440588

typedef struct
{
uint8_t RTC_Hours;
uint8_t RTC_Minutes;
uint8_t RTC_Seconds;
uint8_t RTC_Date;
uint8_t RTC_Wday;
uint8_t RTC_Month;
uint16_t RTC_Year;
} RTC_DateTimeTypeDef;


extern volatile RTC_DateTimeTypeDef currentDateTime;
extern volatile uint32_t RTC_counter;

uint32_t RTC_GetRTC_Counter(void);
void RTC_GetDateTime(uint32_t RTC_counter, volatile RTC_DateTimeTypeDef *RTC_DateTimeStruct);
void RTC_GetLogFormat(volatile RTC_DateTimeTypeDef* RTC_DateTimeStruct, char *buffer);//[2026-07-02 08:01:03]
void RTC_GetMyFormat(volatile RTC_DateTimeTypeDef* RTC_DateTimeStruct, char *buffer);

#endif

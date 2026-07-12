#include "rtc_functions.h"
#include <string.h>
#include <stdio.h>
//#include <stdbool.h>

volatile RTC_DateTimeTypeDef currentDateTime;


uint32_t RTC_GetRTC_Counter(void)
{
		uint8_t a;
		uint16_t y;
		uint8_t m;
		uint32_t JDN;
		a=(14-currentDateTime.RTC_Month)/12;
		y=currentDateTime.RTC_Year+4800-a;
		m=currentDateTime.RTC_Month+(12*a)-3;
		JDN=currentDateTime.RTC_Day;
		JDN+=(153*m+2)/5;
		JDN+=365*y;
		JDN+=y/4;
		JDN+=-y/100;
		JDN+=y/400;
		JDN = JDN -32045;
		JDN = JDN - JULIAN_DATE_BASE;
		JDN*=86400;
		JDN+=(currentDateTime.RTC_Hours*3600);
		JDN+=(currentDateTime.RTC_Minutes*60);
		JDN+=(currentDateTime.RTC_Seconds);
	
		return JDN;
}

void RTC_GetDateTime(uint32_t RTC_counter)
{
		unsigned long time;
		unsigned long t1, a, b, c, d, e, m;
		int year = 0;
		int mon = 0;
		int wday = 0;
		int mday = 0;
		int hour = 0;
		int min = 0;
		int sec = 0;
		uint64_t jd = 0;;
		uint64_t jdn = 0;
		jd = ((RTC_counter+43200)/(86400>>1)) + (2440587<<1) + 1;
		jdn = jd>>1;
		time = RTC_counter;
		t1 = time/60;
		sec = time - t1*60;
		time = t1;
		t1 = time/60;
		min = time - t1*60;
			time = t1;
		t1 = time/24;
		hour = time - t1*24;
		wday = jdn%7;
		a = jdn + 32044;
		b = (4*a+3)/146097;
		c = a - (146097*b)/4;
		d = (4*c+3)/1461;
		e = c - (1461*d)/4;
		m = (5*e+2)/153;
		mday = e - (153*m+2)/5 + 1;
		mon = m + 3 - 12*(m/10);
		year = 100*b + d - 4800 + (m/10);
		currentDateTime.RTC_Year = year;
		currentDateTime.RTC_Month = mon;
		currentDateTime.RTC_Day = mday;
		currentDateTime.RTC_Hours = hour;
		currentDateTime.RTC_Minutes = min;
		currentDateTime.RTC_Seconds = sec;
		currentDateTime.RTC_Wday = wday;
}
//[2026-07-02 08:01:03]
char* RTC_get_format_date(volatile RTC_DateTimeTypeDef* date_time)
{
		static char date_buffer[FLASH_BUFFER_SIZE]; 
		// Сначала заполняем дату
		snprintf(date_buffer, FLASH_BUFFER_SIZE,
                  "[%04d-%02d-%02d %02d:%02d:%02d]",
                  date_time->RTC_Year,
                  date_time->RTC_Month,
                  date_time->RTC_Day,
                  date_time->RTC_Hours,
                  date_time->RTC_Minutes,
                  date_time->RTC_Seconds);

		return date_buffer;
}


void RTC_GetMyFormat(volatile RTC_DateTimeTypeDef* RTC_DateTimeStruct, char *buffer)
{
		const char WDAY0[] = "Monday";
		const char WDAY1[] = "Tuesday";
		const char WDAY2[] = "Wednesday";
		const char WDAY3[] = "Thursday";
		const char WDAY4[] = "Friday";
		const char WDAY5[] = "Saturday";
		const char WDAY6[] = "Sunday";
		const char * WDAY[7]={WDAY0, WDAY1, WDAY2, WDAY3, WDAY4,
		WDAY5, WDAY6};
		const char MONTH1[] = "January";
		const char MONTH2[] = "February";
		const char MONTH3[] = "March";
		const char MONTH4[] = "April";
		const char MONTH5[] = "May";
		const char MONTH6[] = "June";
		const char MONTH7[] = "July";
		const char MONTH8[] = "August";
		const char MONTH9[] = "September";
		const char MONTH10[] = "October";
		const char MONTH11[] = "November";
		const char MONTH12[] = "December";
		const char * MONTH[12]={MONTH1, MONTH2, MONTH3, MONTH4,
		MONTH5, MONTH6, MONTH7, MONTH8, MONTH9, MONTH10, MONTH11, MONTH12};

		sprintf(buffer, "%s %d %s %04d",
		WDAY[RTC_DateTimeStruct->RTC_Wday],
		RTC_DateTimeStruct->RTC_Day,
		MONTH[RTC_DateTimeStruct->RTC_Month -1],
		RTC_DateTimeStruct->RTC_Year);
}


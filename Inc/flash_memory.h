#ifndef __FLASH_MEMORY_H
#define __FLASH_MEMORY_H

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#define MAX_TEMP 26
#define MIN_TEMP 15
#define MAX_PRESS 765
#define MIN_PRESS 750
#define MAX_HUMI 72
#define MIN_HUMI 40

extern volatile uint16_t line_count;
extern volatile uint8_t flash_page_number;

const char *Get_current_date_time(void);
char *Get_temperature_log(int16_t value);
char *Get_humidity_log(int16_t value);
char *Get_pressure_log(int16_t value);
void Is_threshold_value(uint8_t type, int16_t value);
void Flash_write_string(const char* str);
void Flash_read_string(char* buffer, uint16_t maxLen, volatile uint8_t flash_page_number);

void Increment_page(void);

#endif

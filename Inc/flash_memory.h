#ifndef __FLASH_MEMORY_H
#define __FLASH_MEMORY_H

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#define MAX_LOG_LENGTH 2048 //Максимально возможная длина лога для выгрузки на ПК
#define MAX_TEMP 30
#define MIN_TEMP 15
#define MAX_PRESS 765
#define MIN_PRESS 742
#define MAX_HUMI 76
#define MIN_HUMI 60

extern volatile uint16_t line_count;
extern volatile uint8_t flash_page_number;
extern volatile uint8_t allow_temp_log;
extern volatile uint8_t allow_humi_log;
extern volatile uint8_t allow_press_log;

const char *Get_current_date_time(void);
char* Get_temperature_log(int16_t value);
char* Get_humidity_log(int16_t value);
char* Get_pressure_log(int16_t value);
void Is_threshold_value(uint8_t type, int16_t value);
void Flash_write_string(const char *str);
void Flash_read_string(char *buffer, uint16_t maxLen, volatile uint8_t flash_page_number);
void Increment_page(void);
uint16_t Read_log_entry(char *buffer, uint32_t address);
uint16_t Read_page_log(char *log_buffer_uart, uint32_t page_address);

#endif

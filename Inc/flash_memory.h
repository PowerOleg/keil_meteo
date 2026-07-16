#ifndef __FLASH_MEMORY_H
#define __FLASH_MEMORY_H

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

#define TIMEOUT_VALUE 28800U								//записывает лог не чаще чем каждые 8 часов
#define FLASH_USER_START_ADDR   0x0800FC00  //адрес начала последней страницы
#define FLASH_START_OF_LAST_PAGE			0x0800F800	//адрес второй страницы от конца
#define LOG_PAGE_SIZE      1024     // –азмер одной страницы в байтах
#define LOG_ENTRY_SIZE     40       // –азмер одной записи лога в байтах
#define LOG_BUFFER_SIZE 2048 //ћаксимально возможна€ длина лога дл€ выгрузки на ѕ 
#define MAX_TEMP 30
#define MIN_TEMP 15
#define MAX_PRESS 765
#define MIN_PRESS 740
#define MAX_HUMI 76
#define MIN_HUMI 60

extern volatile uint32_t entry_idx;
extern volatile uint8_t flash_page_number;
extern volatile uint8_t allow_temp_log;
extern volatile uint8_t allow_humi_log;
extern volatile uint8_t allow_press_log;

const char *Get_current_date_time(void);
char* Get_temperature_log(int16_t value);
char* Get_humidity_log(int16_t value);
char* Get_pressure_log(int16_t value);
void Is_threshold_value(uint8_t type, int16_t value);
void Get_last_entry_idx(void);
void Flash_write_string(const char *str);
void Flash_read_string(char *buffer, uint16_t maxLen, volatile uint8_t flash_page_number);
uint16_t Read_log_entry(char *buffer, uint32_t address);
uint16_t Read_page_log(char *log_buffer_uart, uint32_t page_address, uint16_t total_bytes_read);
uint16_t Get_log(char *log_buffer_uart);
void Delete_flash_log(void);

#endif

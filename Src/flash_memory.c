#include "flash_memory.h"
#include "common.h"
#include "rtc_functions.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart.h"

#define FLASH_USER_START_ADDR   0x0800FC00  //адрес начала последней страницы
#define START_OF_LAST_PAGE			0x0800F400
//#define FLASH_PAGE_SIZE         1024
//#define OFFSET 									0x28
volatile uint16_t line_count = 0;

//temp 30 [2026-07-02 08:01:03]
//temp 15 [2026-07-02 08:01:03]
//press 755 [2026-07-02 08:01:03]
//press 765 [2026-07-02 08:01:03]
//humi 40 [2026-07-02 08:01:03]
//humi 76 [2026-07-02 08:01:03]


/*const char *Get_current_date_time(void)
{
    static char buffer[] = "[2026-07-02 08:01:03]"; // Здесь подставьте вашу реализацию получения текущей даты и времени
    return buffer;
}*/



// Формирование строки лога температуры
char *Get_temperature_log(int16_t value)
{
    static char log_buffer[BUFFER_SIZE];
		RTC_GetLogFormat(&currentDateTime, log_buffer);
    snprintf(log_buffer, sizeof(log_buffer), "temp %d %s", value, log_buffer);
		Uart2_send_string(log_buffer);																													//delete
    return log_buffer;
}

// Формирование строки лога влажности
char *Get_humidity_log(int16_t value)
{
    static char log_buffer[BUFFER_SIZE];
		RTC_GetLogFormat(&currentDateTime, log_buffer);
    snprintf(log_buffer, sizeof(log_buffer), "humi %d %s", value, log_buffer);
		Uart2_send_string(log_buffer);																													//delete
    return log_buffer;
}

// Формирование строки лога давления
char *Get_pressure_log(int16_t value)
{
    static char log_buffer[BUFFER_SIZE];
		RTC_GetLogFormat(&currentDateTime, log_buffer);
    snprintf(log_buffer, sizeof(log_buffer), "press %d %s", value, log_buffer);
		Uart2_send_string(log_buffer);																													//delete
    return log_buffer;
}

// Проверка превышения порогов и запись в память
void Is_threshold_value(uint8_t type, int16_t value)
{
    switch (type) {
        case TEMPERATURE:
            if ((value > MAX_TEMP) || (value < MIN_TEMP))
						{
                Flash_write_string(Get_temperature_log(value));
            }
            break;
            
        case HUMIDITY:
            if ((value > MAX_HUMI) || (value < MIN_HUMI))
						{
                Flash_write_string(Get_humidity_log(value));
            }
            break;
            
        case PRESSURE:
            if ((value > MAX_PRESS) || (value < MIN_PRESS))
						{
                Flash_write_string(Get_pressure_log(value));
            }
            break;
    }
}

// Функция записи строки во Flash
/*void Flash_write_string(const char* str)
{
    uint32_t addr = FLASH_USER_START_ADDR + (line_count * OFFSET);
    uint16_t data;
    int len = strlen(str) + 1; // с нуль-терминатором
    int i;
    
    FLASH_Unlock();
    FLASH_ErasePage(FLASH_USER_START_ADDR);
    
    // Пишем полусловами (16 бит), собирая байты попарно
    for (i = 0; i < len; i += 2) {
        data = (uint16_t)str[i];
        if (i + 1 < len)
            data |= (uint16_t)str[i + 1] << 8;
        FLASH_ProgramHalfWord(addr, data);
        addr += 2;
    }
    
    FLASH_Lock();
//		line_count++;
}*/

// Функция чтения строки из Flash
void Flash_read_string(char* buffer, uint16_t maxLen)
{
    uint32_t addr = FLASH_USER_START_ADDR;// + (line_count * OFFSET);
    uint16_t data;
    int i = 0;

    while (i < maxLen - 1) {
        data = *(__IO uint16_t*)addr;
        buffer[i] = (char)(data & 0xFF);
        if (buffer[i] == '\0') break;
        i++;
        if (i >= maxLen - 1) break;
        buffer[i] = (char)(data >> 8);
        if (buffer[i] == '\0') break;
        i++;
        addr += 2;
    }
    buffer[i] = '\0';
}

#define LOG_PAGE_SIZE      1024     // Размер страницы в байтах
#define LOG_ENTRY_SIZE     40       // Размер одного лога в байтах
#define LOG_ENTRIES_PER_PAGE (LOG_PAGE_SIZE / LOG_ENTRY_SIZE) // Количество логов на страницу

static uint32_t page_addr = FLASH_USER_START_ADDR; // Текущая страница
static uint32_t entry_idx = 0;                     // Индекс текущего лога в странице

void Flash_write_string(const char* str)
{
    uint32_t addr = page_addr + (entry_idx * LOG_ENTRY_SIZE);
    uint16_t data;
    int len = strlen(str) + 1; // Длина строки с нулевым символом
		
		 // Увеличение индекса записи
    entry_idx++;
	
		FLASH_Unlock();
    // Если достигли конца страницы, переходим к следующей
		if (entry_idx >= LOG_ENTRIES_PER_PAGE)
		{
    entry_idx = 0;
    // Переход к предыдущей странице
    page_addr -= LOG_PAGE_SIZE;
    
    if (page_addr < START_OF_LAST_PAGE)
        page_addr = FLASH_USER_START_ADDR;// Вернуться к последней странице, если вышли за нижнюю границу
    
		        // Стереть новую страницу
 //       FLASH_Unlock();
        FLASH_ErasePage(page_addr);
//        FLASH_Lock();
		}
    
    // Записываем лог
//    FLASH_Unlock();
    for (int i = 0; i < len; i += 2) {
        data = (uint16_t)str[i];
        if (i + 1 < len)
            data |= (uint16_t)str[i + 1] << 8;
        FLASH_ProgramHalfWord(addr, data);
        addr += 2;
    }
    FLASH_Lock();
}

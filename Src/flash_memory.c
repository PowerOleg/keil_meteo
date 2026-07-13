#include "flash_memory.h"
#include "common.h"
#include "rtc_functions.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart.h"

#define TIMEOUT_VALUE 28800U								//записывает лог не чаще чем каждые 8 часов
#define FLASH_USER_START_ADDR   0x0800FC00  //адрес начала последней страницы
#define START_OF_LAST_PAGE			0x0800F400	//адрес последней страницы при условии что нумерация страниц от идет конца к началу

#define LOG_PAGE_SIZE      1024     // Размер страницы в байтах
#define LOG_ENTRY_SIZE     40       // Размер одной записи лога в байтах
#define LOG_ENTRIES_PER_PAGE (LOG_PAGE_SIZE / LOG_ENTRY_SIZE) // Количество записей на страницу

volatile uint16_t line_count = 0;
volatile uint8_t allow_temp_log = 1;
volatile uint8_t allow_humi_log = 1;
volatile uint8_t allow_press_log = 1;

// Глобальные переменные для хранения времени последней записи
volatile uint32_t last_temp_log_time = 0;
volatile uint32_t last_humi_log_time = 0;
volatile uint32_t last_press_log_time = 0;

static uint32_t page_addr = FLASH_USER_START_ADDR;
static uint32_t entry_idx = 0;// Индекс текущей записи лога на странице


// Глобальные переменные для отслеживания состояния страниц
volatile uint8_t flash_page_number = 1; 

// Формирование строки лога температуры T v26v [y2026ym07md02dh08hm01ms03s]
char *Get_temperature_log(int16_t value)
{
    static char log_buffer[FLASH_BUFFER_SIZE];
    snprintf(log_buffer, sizeof(log_buffer), "T %d %s", value, RTC_get_format_date(&currentDateTime));
//		Uart2_send_string(log_buffer);																													//delete
    return log_buffer;
}

// Формирование строки лога влажности
char *Get_humidity_log(int16_t value)
{
    static char log_buffer[FLASH_BUFFER_SIZE];
    snprintf(log_buffer, sizeof(log_buffer), "H %d %s", value, RTC_get_format_date(&currentDateTime));
//		Uart2_send_string(log_buffer);																													//delete
    return log_buffer;
}

// Формирование строки лога давления
char *Get_pressure_log(int16_t value)
{
    static char log_buffer[FLASH_BUFFER_SIZE];
    snprintf(log_buffer, sizeof(log_buffer), "P %d %s", value, RTC_get_format_date(&currentDateTime));
//		Uart2_send_string(log_buffer);																													//delete
    return log_buffer;
}

static void Reset_logging(void)
{
		allow_temp_log = 1;
		allow_humi_log = 1;
		allow_press_log = 1;
}
static uint8_t Is_timeout(volatile uint32_t *last_temp_log_time)
{
		uint32_t current_time = RTC_GetRTC_Counter();
		if (current_time - *last_temp_log_time >= TIMEOUT_VALUE)
		{
				*last_temp_log_time = current_time;
				return 1;
    }
		return 0;
}

// Проверка превышения порогов и запись в память
void Is_threshold_value(uint8_t type, int16_t value)
{
    switch (type) {
        case TEMPERATURE:
						allow_temp_log = Is_timeout(&last_temp_log_time);
            if (((value > MAX_TEMP) || (value < MIN_TEMP)) && allow_temp_log)
						{
                Flash_write_string(Get_temperature_log(value));
								allow_temp_log = 0;
            }
            break;
        case HUMIDITY:
						allow_humi_log = Is_timeout(&last_humi_log_time);
            if (((value > MAX_HUMI) || (value < MIN_HUMI)) && allow_humi_log)
						{
                Flash_write_string(Get_humidity_log(value));
								allow_humi_log = 0;
            }
            break;
        case PRESSURE:
						allow_press_log = Is_timeout(&last_press_log_time);
            if (((value > MAX_PRESS) || (value < MIN_PRESS)) && allow_press_log)
						{
                Flash_write_string(Get_pressure_log(value));
								allow_press_log = 0;
            }
            break;
    }
}


/**
 * @brief Функция чтения строки из Flash-памяти
 *
 * @param buffer Указатель на буфер для чтения данных
 * @param maxLen Максимальная длина буфера
 */
void Flash_read_string(char* buffer, uint16_t maxLen, volatile uint8_t flash_page_number)
{
    uint32_t addr = page_addr + ((flash_page_number - 1) * 0x28);
    uint16_t data;
    int i = 0;

    while (i < maxLen - 1 && addr <= FLASH_USER_START_ADDR + LOG_PAGE_SIZE) {
        data = *(__IO uint16_t*)addr;
        buffer[i++] = (char)(data & 0xFF);
        if (buffer[i - 1] == '\0') break;
        if (i >= maxLen - 1) break;
        buffer[i++] = (char)(data >> 8);
        if (buffer[i - 1] == '\0') break;
        addr += 2;
    }
    buffer[i] = '\0'; // Завершаем строку нулевым символом
}

/**
 * @brief Функция инкрементации страницы
 */
void Increment_page(void)
{
    flash_page_number++; // Инкрементируем номер страницы
    if (flash_page_number > 3) {
        flash_page_number = 1; // Циклический возврат к первой странице
    }
    page_addr = FLASH_USER_START_ADDR - (flash_page_number - 1) * LOG_PAGE_SIZE;
}


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



volatile uint32_t current_page_address = FLASH_USER_START_ADDR;
volatile uint32_t log_length = 0;



/**
 * @brief Если хотя бы одно полуслово отличается от значения 0xFFFF, функция немедленно возвращает false, сигнализируя о том, что страница не пуста.
 *
 * @param page_start Адрес начала страницы
 * @return true если страница пустая
 */
static bool Is_page_empty(uint32_t page_start)
{
    const uint16_t empty_word = 0xFFFF; // Значение пустого полуслова в флеш-памяти
    const size_t words_per_page = LOG_PAGE_SIZE / sizeof(uint16_t); // Количество полуслов на странице

    // Проверяем каждое полуслово на странице
    for (size_t i = 0; i < words_per_page; ++i) {
        if (*(uint16_t *)(page_start + i * sizeof(uint16_t)) != empty_word) {
            return false; // Найдено непустое полуслово
        }
    }

    return true; // Вся страница пуста
}

/**
 * @brief Чтение одной записи лога из флеш-памяти
 *
 * @param buffer Указатель на буфер для сохранения результата
 * @param address Текущий адрес чтения
 * @return Количество прочитанных байтов
 */
uint16_t Read_log_entry(char* buffer, uint32_t address) {
    uint16_t bytes_read = 0;
    uint16_t word_data;

    while (bytes_read < LOG_ENTRY_SIZE) {
        word_data = *(uint16_t*)address;
        buffer[bytes_read++] = (char)(word_data & 0xFF); // Младший байт
        if (buffer[bytes_read - 1] == ']') {
            buffer[bytes_read] = '\0';
            return bytes_read;
        }

        if (bytes_read < LOG_ENTRY_SIZE) {
            buffer[bytes_read++] = (char)(word_data >> 8); // Старший байт
            if (buffer[bytes_read - 1] == ']') {
                buffer[bytes_read] = '\0';
                return bytes_read;
            }
        }

        address += 2;
    }

    buffer[bytes_read] = '\0';
    return bytes_read;
}

/**
 * @brief Функция чтения из флеш-памяти и формирования текста для одной страницы размером максимум 1024 байт
 *
 * @return Размер полученного лога в байтах
 */
uint16_t Read_page_log(char *log_buffer_uart, uint32_t page_address) {
    uint16_t total_bytes_read = 0;

    if (Is_page_empty(page_address)) {
        return 0;
    }

    uint32_t entry_address = page_address;
		uint8_t entry_count = 0;
    while (entry_address < page_address + LOG_PAGE_SIZE)
		{
        char temp_buffer[LOG_ENTRY_SIZE];
        uint16_t bytes_read = Read_log_entry(temp_buffer, entry_address);

        if (bytes_read == 0 || temp_buffer[0] == '\0') {
            break;
        }

        if (++entry_count >= LOG_ENTRIES_PER_PAGE)
            break;

        memcpy(log_buffer_uart + total_bytes_read, temp_buffer, bytes_read);//strcpy(log_buffer_uart + total_bytes_read, temp_buffer);
        total_bytes_read += bytes_read;

        // Добавление разделителя \r\n
        log_buffer_uart[total_bytes_read++] = '\r';
        log_buffer_uart[total_bytes_read++] = '\n';

        entry_address += LOG_ENTRY_SIZE;
				temp_buffer[bytes_read] = '\r';
				temp_buffer[bytes_read + 1] = '\n';
				temp_buffer[bytes_read + 2] = '\0';
				Uart2_send_string(temp_buffer);
    }

    return total_bytes_read;
}
/*uint16_t Read_page_log(char *log_buffer_uart, uint32_t page_address)
{
//    uint32_t page_address = FLASH_USER_START_ADDR;//current_page_address;
    uint16_t total_bytes_read = 0;

//    while (page_address >= START_OF_LAST_PAGE) {
        if (Is_page_empty(page_address))
						return 0;

        uint32_t entry_address = page_address;
        //bool found_end_marker = false;

        while (entry_address < page_address + LOG_PAGE_SIZE)
				{
            static char temp_buffer[LOG_PAGE_SIZE];
            uint16_t bytes_read = Read_log_entry(temp_buffer, entry_address);
            
						if (bytes_read == 0xFFFF)
								break;
            
					


            strcpy(log_buffer_uart + total_bytes_read, temp_buffer);
            total_bytes_read += bytes_read;

           

            entry_address += LOG_ENTRY_SIZE;
        }

//        page_address -= LOG_PAGE_SIZE;
//    }

    return total_bytes_read;
}*/

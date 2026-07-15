#include <string.h>
#include <stdio.h>
#include "common.h"
#include "hw_config.h"
#include "rtc_functions.h"
#include "led.h"
#include "uart.h"
#include "oled_display.h"
#include "timer.h"
#include "bme280.h"
#include "keypad4x4.h"
#include "flash_memory.h"

volatile uint8_t cur_action = 0;
volatile uint8_t previous_action = 0;

volatile uint8_t initial_set_up = 1;
volatile uint8_t pressed_key = 0;
uint8_t time_indices[] = {0, 0, 10, 0, 0};// время в формате: 12:34
char flash_buff[40];
char log_buffer_uart[LOG_BUFFER_SIZE] = {'0'}; //Буфер для временного хранения лога

Led led_a8;
Led led_c13;

void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
    {
				RTC_ClearITPendingBit(RTC_IT_SEC);
        RTC_GetDateTime(RTC_GetCounter());
    }
}

void Set_up_time_and_date(uint8_t *time, uint8_t *date)
{
		pressed_key = Check_keypad_pressed();
		if (symbol_index > 4)
		{
				Input_date(date, pressed_key);					
				OLED_ClearBuffer();
				OLED_PrintScaledSymbols(0, 0, font_table, init_message_line0, 8, 2);
				OLED_PrintScaledSymbols(0, 15, font_table, init_message_line1, 5, 2);							
				OLED_PrintScaledSymbols(70, 15, font_table, time, 5, 2);
				OLED_PrintScaledSymbols(0, 30, font_table, init_message_line2, 4, 2);	
				OLED_PrintScaledSymbols(0, 45, font_table, date, 11, 2);
				OLED_UpdateScreen();
		}
						
		if (symbol_index >= 0 && symbol_index <= 4)
		{
				Input_time(time, pressed_key);
				OLED_ClearBuffer();
				OLED_PrintScaledSymbols(0, 0, font_table, init_message_line0, 8, 2);
				OLED_PrintScaledSymbols(0, 15, font_table, init_message_line1, 5, 2);							
				OLED_PrintScaledSymbols(70, 15, font_table, time, 5, 2);
				OLED_UpdateScreen();
		}
						
		if (symbol_index > 14)
		{
				uint8_t hours = (time[0] * 10) + time[1]; 
				uint8_t minutes = (time[3] * 10) + time[4];
				uint8_t day = (date[0] * 10) + date[1];
				uint8_t month = (date[3] * 10) + date[4];
				uint16_t year = (uint16_t)(date[6] * 1000) + (uint16_t)(date[7] * 100) + (uint16_t)(date[8] * 10) + date[9];
			
				RTC_init_lse(year, month, day, hours, minutes, 0);
				cur_action = TIME;
				initial_set_up = 0;
		}
}


int main(void)
{
		if (Clock_config_72mhz() == ERROR)
			Error_handler();

		RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1), ENABLE);
		RCC_APB1PeriphClockCmd((RCC_APB1Periph_USART2 | RCC_APB1Periph_PWR | RCC_APB1Periph_BKP), ENABLE);
		Led_init(&led_a8, GPIOA, GPIO_Pin_8);
		Led_init(&led_c13, GPIOC, GPIO_Pin_13);
		Tim3_init_10sec_timer();
		Init_systick_us();
		Tim2_count_mode_up();
		
		//keypad 4x4
		Keypad_init_gpio();

//	UART2
		Uart2_init();//require Tim2

		//SPI
	  SPI1_common_gpio_init();
		Delay_us(100000);
		SPI1_common_init();//Oled_spi_init();
		Delay_us(100000);
				
		BME280_gpio_init();
		Delay_us(100000);
		//OLED DISPLAY
		Oled_gpio_init();
		Delay_us(100000);
		OLED_Init_SSD1306();
		Delay_us(100000);
    OLED_ClearBuffer();
	
		//BME280
		SPI_clear_rxne();
		BME280_init();
		BME280_RawData_t raw_data;
		BME280_Result_t bmp280_result;
		Delay_us(10000);
	
		//check led
		Led_toggle(&led_a8);
		Delay_us(500000);
		Led_toggle(&led_c13);
		Led_toggle(&led_a8);

		
		uint8_t time[TIME_SIZE] = {0};//08:01
		uint8_t date[10] = {0};//04.07.2026
		time[2] = 10;
		date[2] = 12;
		date[5] = 12;		
		
		while(1)
		{
				Delay_us(10000);
				if (initial_set_up)
				{
						Set_up_time_and_date(time, date);
						continue;
				}
				
				if (tim3_10sec_flag)
				{
						tim3_10sec_flag = 0;
						BME280_measure(&raw_data);
						BME280_compensate(&raw_data, &bmp280_result);
						Is_threshold_value(TEMPERATURE, bmp280_result.temperature_c);
						Is_threshold_value(HUMIDITY, bmp280_result.humidity);
						Is_threshold_value(PRESSURE, bmp280_result.pressure_mm_rt_st);
				}
					
				pressed_key = Check_keypad_pressed();
				if (pressed_key != NO_KEY)
						cur_action = pressed_key;

				switch (cur_action)
				{
						case TIME:
								OLED_ClearBuffer();
								OLED_PrintScaledSymbols(10, 0, font_table, vremya_indices, 6, 2);					
								OLED_fill_indices(time_indices, currentDateTime.RTC_Hours, currentDateTime.RTC_Minutes);
								OLED_PrintScaledSymbols(10, 30, font_table, time_indices, 5, 3);
								OLED_UpdateScreen();
								break;
						case TEMPERATURE:
								OLED_ClearBuffer();
								OLED_PrintScaledSymbols(10, 0, font_table, temperature_indices, 8, 2);
								OLED_PrintTemperature(10, 30, bmp280_result.temperature_c, 3, font_table);
								OLED_UpdateScreen();
								break;
						case HUMIDITY:
								OLED_ClearBuffer();
								OLED_PrintScaledSymbols(10, 0, font_table, humidity_indices, 10, 2);
								OLED_PrintHumidity(10, 30, bmp280_result.humidity, 3, font_table);
								OLED_DrawPercent15x21Buf(92, 4);
								OLED_UpdateScreen();
								break;
						case PRESSURE:
								OLED_ClearBuffer();
								OLED_PrintScaledSymbols(10, 0, font_table, pressure_indices, 9, 2);
								OLED_PrintPressure(24, bmp280_result.pressure_mm_rt_st, 3, font_table);
								OLED_UpdateScreen();
								break;
						case MIN_MAX_LOG:
								OLED_ClearBuffer();
								Flash_read_string(flash_buff, 0x28, flash_page_number);
								Display_flash_data(flash_buff, flash_page_number, 1);
								Flash_read_string(flash_buff, 0x28, flash_page_number + 1);
								Display_flash_data(flash_buff, flash_page_number + 1, 2);
								Flash_read_string(flash_buff, 0x28, flash_page_number + 2);
								Display_flash_data(flash_buff, flash_page_number + 2, 3);
								OLED_UpdateScreen();
								break;
						case PAGE_UP:
								flash_page_number++;
								cur_action = previous_action;
								break;
						case PAGE_DOWN:
								flash_page_number--;
								cur_action = previous_action;
								break;
						case SEND_DATA_TO_PC:
						{
								memset(log_buffer_uart, 0, LOG_BUFFER_SIZE);
								uint16_t total_page_size = Read_page_log(log_buffer_uart, FLASH_USER_START_ADDR, 0);
								if (total_page_size >= LOG_PAGE_SIZE - 100)
								{
										uint16_t second_page_size = Read_page_log(log_buffer_uart, START_OF_LAST_PAGE, total_page_size);
										if (second_page_size > 0)
												total_page_size += second_page_size;
								}
								
								if (total_page_size > 0)
								{
										Uart2_send_string(log_buffer_uart, total_page_size);
										while(transmission_in_progress) {}
										OLED_ClearBuffer();
										OLED_PrintScaledSymbols(10, 0, font_table, sent_indices, 8, 2);
										OLED_UpdateScreen();
										Delay_us(1000000);
								}
								cur_action = previous_action;
								break;
						}

				}
				previous_action = cur_action;
		}
	
}

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


#define TIME 0x31
#define TEMPERATURE 0x32
#define HUMIDITY 0x33
#define PRESSURE 0x34
#define MIN_MAX_LOG 0x35
#define SEND_DATA_TO_PC 0x36

volatile uint8_t cur_action = 0;

const uint8_t *font_table[] = {
    digit_font_5x7[0], digit_font_5x7[1], digit_font_5x7[2],
    digit_font_5x7[3], digit_font_5x7[4], digit_font_5x7[5],
    digit_font_5x7[6], digit_font_5x7[7], digit_font_5x7[8],
    digit_font_5x7[9],
    colon_5x7,          // 10
    minus_5x7,          // 11
    dot_5x7,            // 12
    degree_5x7,         // 13
    letter_C_5x7,       // 14
		percent_5x7,				// 15
		exclamation_5x7,		// 16
		letter_m_5x7,				// 17
		letter_p_5x7,				// 18
		letter_T_cyr_5x7,		// 19
		letter_V_cap_5x7,		// 20
		letter_E_cap_5x7,		// 21
		letter_M_cap_5x7,		// 22
		letter_Ya_cap_5x7,	// 23
		letter_ru_P_cap_5x7,// 24
		letter_A_cap_5x7,		// 25
		hyphen_5x7,					// 26
		letter_L_cap_5x7,		// 27
		letter_Zh_cap_5x7,	// 28
		letter_N_cap_5x7,		// 29
		letter_O_cap_5x7,		// 30
		letter_SoftSign_5x7,// 31
		letter_D_cap_5x7,		// 32
		letter_I_cap_5x7,		// 33
		letter_U_cap_5x7,		// 34
		space_5x7,					// 35
		slash_5x7						// 36
};

const uint16_t row_pins[ROWS] = {GPIO_Pin_11, GPIO_Pin_10, GPIO_Pin_1, GPIO_Pin_0};
const uint16_t col_pins[COLS] = {GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15};

// Индексы: В(20), Р(18), Е(21), M(22), Я(23), :(10)
const uint8_t vremya_indices[] = {20, 18, 21, 22, 23, 10};
// Индексы: Т(19), Е(21), M(22), П(24), -(26), Р(18), А(25), :(10)
const uint8_t temperature_indices[] = {19, 21, 22, 24, 26, 18, 25, 10};
// Индексы: В(20), Л(27), А(25), Ж(28), Н(29), О(30), С(14), Т(19), Ь(31), :(10)
const uint8_t humidity_indices[] = {20, 27, 25, 28, 29, 30, 14, 19, 31, 10};
// Индексы: Д(32), А(25), В(20), Л(27), Е(21), Н(29), И(33), Е(21), :(10)
const uint8_t pressure_indices[] = {32, 25, 20, 27, 21, 29, 33, 21, 10};
// Индексы: В(20), В(20), Е(21), Д(32), И(33), Т(19), Е(21) 
const uint8_t init_message_line0[] = {20, 20, 21, 32, 33, 19, 21, 35};
// Д(32), А(25), Т(19), У(34), пробел(35), В(20), Р(18), Е(21), M(22), Я(23)
//const uint8_t init_message_line1[] = {32, 25, 19, 34, 36, 20, 18, 21, 22, 23, 10};
const uint8_t init_message_line1[] = {20, 18, 21, 22, 23};
// Д(32), А(25), Т(19), У(34),
const uint8_t init_message_line2[] = {32, 25, 19, 34};

volatile uint8_t initial_set_up = 1;
volatile uint8_t pressed_key = 0;
uint8_t time_indices[] = {0, 0, 10, 0, 0};// время в формате: 12:34

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
		pressed_key = Check_keypad_pressed(row_pins, col_pins);
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

				uint8_t hours = (time[0] * 10) + time[1];//(uint8_t)strtol((const char *)&time[0], NULL, 10);// 
				uint8_t minutes = (time[3] * 10) + time[4];//(uint8_t)strtol((const char *)&time[3], NULL, 10);//(time[3] * 10) + time[4];
				uint8_t day = (date[0] * 10) + date[1]; //(uint8_t)strtol(&date_char[0], NULL, 10);
				uint8_t month = (date[3] * 10) + date[4]; //(uint8_t)strtol(&date_char[3], NULL, 10);
				uint16_t year = (uint16_t)(date[6] * 1000) + (uint16_t)(date[7] * 100) + (uint16_t)(date[8] * 10) + date[9];//(uint16_t)strtol((const char *)&date[6], NULL, 10);//(date[6] - '0') * 1000 + (date[7] - '0') * 100 + (date[8] - '0') * 10 + (date[9] - '0');

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
		// Назначаем пины к строкам и столбцам клавитуры
		Keypad_init_gpio(row_pins, col_pins);

//	UART2
//	Uart2_init();//require Tim2
//	Uart2_send_string("UART2 init");

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

	
	//FLASH
/*	char test_string[] = "1234";
	char read_back[100];
	Flash_write_string(test_string);
  Flash_read_string(read_back, sizeof(read_back));*/
	

	


		
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

		//08:01 04.07.2026);
		uint8_t time[TIME_SIZE] = {0};
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
				
				//every 10 sec gets measure from BME280
				if (tim3_10sec_flag)
				{
						tim3_10sec_flag = 0;
						BME280_Measure(&raw_data);
						BME280_Compensate(&raw_data, &bmp280_result);
				}
					
				pressed_key = Check_keypad_pressed(row_pins, col_pins);
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
						default:
								break;
				}
		}
	
}

#include <string.h>
#include <stdio.h>
#include "common.h"
#include "hw_config.h"
#include "rtc_functions.h"
#include "led.h"
#include "uart.h"
#include "lcd1602.h"
#include "oled_display.h"
#include "timer.h"
//#include "bmp280.h"
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
		letter_C_5x7				// 20
};


volatile uint8_t lcd_cycle_count = 31;
Led led_a8;
Led led_c13;
	
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_SEC);   // сброс флага обязательно!
        RTC_counter = RTC_GetCounter();
        RTC_GetDateTime(RTC_counter, &currentDateTime);
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
		RTC_init_lse(2026, 7, 4, 8, 1, 0);




//keypad 4x4
		// Назначаем пины к строкам и столбцам клавитуры
		const uint16_t row_pins[ROWS] = {GPIO_Pin_11, GPIO_Pin_10, GPIO_Pin_1, GPIO_Pin_0};
		const uint16_t col_pins[COLS] = {GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15};
		Keypad_init_gpio(row_pins, col_pins);

	

//	UART2
//	Uart2_init();//require Tim2
//	Uart2_send_string("UART2 init");

//	I2C
/*	I2C1_gpio_init();
	Delay_us(200000);
	I2C1_Init();
	Delay_us(200000);
*/

	//SPI
/*	SPI1_gpio_init();
	Tim3_Delay_ms(200);
	SPI1_init();
	Tim3_Delay_ms(200);*/
	


	//LCD
/*
	Lcd_init();
	Tim3_Delay_ms(200);
	Lcd_clear();
	Tim3_Delay_ms(200);
*/
/*	Lcd_set_cursor_position(0, 0);
	strncpy(lcd_buffer, "BMP280", 6);
	Lcd_print((uint8_t *)lcd_buffer, strlen(lcd_buffer));
	memset(lcd_buffer, ' ', LCD_LINE_SIZE);
	//					snprintf(lcd_buffer, LCD_LINE_SIZE, "%.2f C", bmp280_result.temperature_c);
//				snprintf(lcd_buffer, LCD_LINE_SIZE, "%u", pressure_rt_st);
	*/

	//SPI
	//OLED DISPLAY
	  SPI1_common_gpio_init();
		Delay_us(100000);
		SPI1_common_init();//Oled_spi_init();
		Delay_us(100000);
				
		BME280_gpio_init();
		Delay_us(100000);
		Oled_gpio_init();
		Delay_us(100000);
		OLED_Init_SSD1306();
		Delay_us(100000);


		uint8_t time_indices[] = {1, 2, 10, 3, 4}; // 1,2,:,3,4
    OLED_ClearBuffer();


		
    // --- Строка "Время:" (scale 2) ---
    // Индексы: В(11), р(12), е(13), м(14), я(15), :(10) (двоеточие такое же)
//    const uint8_t vremya_indices[] = {11, 12, 13, 14, 15, 10}; // "Время:"
//    OLED_PrintScaledString(0, 0, font_table, vremya_indices, 6, 2);
	
	//FLASH
/*	char test_string[] = "1234";
	char read_back[100];
	Flash_write_string(test_string);
  Flash_read_string(read_back, sizeof(read_back));*/
	

	


		
		//BME280
		SPI_Clear_RXNE();
		BME280_init();
		BME280_RawData_t raw_data;
		BME280_Result_t bmp280_result;
		Delay_us(10000);
	
						//check led
		Led_toggle(&led_a8);
		Delay_us(500000);
		Led_toggle(&led_c13);
		Led_toggle(&led_a8);

		
		while(1)
		{
				Delay_us(20000);
//				Led_toggle(&led_a8);

				//BME280
				if (tim3_10sec_flag)
				{
						tim3_10sec_flag = 0;
						BME280_Measure(&raw_data);
						BME280_Compensate(&raw_data, &bmp280_result);
				}	
					
				uint8_t pressed_key = Check_keypad_pressed(row_pins, col_pins);
				if (pressed_key != NO_KEY)
						cur_action = pressed_key;

				switch (cur_action)
				{
						case TIME:
								OLED_ClearBuffer();
								OLED_fill_indices(time_indices, currentDateTime.RTC_Hours, currentDateTime.RTC_Minutes);
								OLED_PrintScaledSymbols(10, 30, font_table, time_indices, 5, 3);
								OLED_UpdateScreen();
								break;
						case TEMPERATURE:
								OLED_ClearBuffer();
								OLED_PrintTemperature(0, 30, bmp280_result.temperature_c, 3, font_table);
								OLED_UpdateScreen();
								break;
						case HUMIDITY:
								OLED_ClearBuffer();
								OLED_PrintHumidity(10, 20, bmp280_result.humidity, 3, font_table);
								OLED_UpdateScreen();
								OLED_DrawPercent15x21(98, 2);
								break;
						case PRESSURE:
								OLED_ClearBuffer();
								OLED_PrintPressure(20, bmp280_result.pressure_mm_rt_st, 3, font_table);
								OLED_UpdateScreen();
								break;
						default:
								break;
				}

	//			Uart2_receive_string();
		}
	
}

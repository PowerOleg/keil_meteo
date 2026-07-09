#include "keypad4x4.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

volatile uint8_t symbol_index = 0;

uint8_t keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

volatile Key_state keys_state[ROWS][COLS];

void Input_time(uint8_t *time, const uint8_t pressed_key)
{
		if (pressed_key != NO_KEY && pressed_key >= 0x30 && pressed_key < 0x40)
		{
				if (symbol_index == 0 && pressed_key > 0x32)
						return;
				if (symbol_index == 1 && time[0] == 2 && pressed_key > 0x33)
						return;
				if (symbol_index == 3 && pressed_key > 0x35)
						return;

				if (symbol_index == 2)
						symbol_index++;
				time[symbol_index++] = pressed_key - '0';
		}
}

void Input_date(uint8_t *date, const uint8_t pressed_key)
{
		if (pressed_key != NO_KEY && pressed_key >= 0x30 && pressed_key < 0x40)
		{
				uint8_t key = pressed_key - '0';
			
				if (symbol_index == 2 + TIME_SIZE || symbol_index == 5 + TIME_SIZE)
						symbol_index++;
				
				uint8_t date_index = symbol_index - TIME_SIZE;
        // Проверка диапазона дня
				if (date_index == 0 && (key < 0 || key > 3))
            return;
        if (date_index == 1 && date[0] == 3 && key > 1)
            return;
				
        // Проверка диапазона месяца
        if (date_index == 3 && key > 1)
            return;
				if (date_index == 4 && date[3] == 1 && key > 2)
						return;
				// Проверка диапазона года. Чтобы влезть в размер счетчика RTC нужно быть больше 1970 и меньше 2200
				if (date_index == 6 && (key < 1 || key > 2))
						return;
				if (date_index == 7 && date[6] == 1 && key < 9)
						return;
				
				if (date_index == 8 && date[6] == 1 && date[7] == 9 && key < 8)
						return;
				
				if (date_index == 7 && date[6] == 2 && key != 0)
						return;
				if (date_index == 8 && date[6] == 2 && date[7] == 1 && key > 8)
						return;

				date[date_index] = key;
				symbol_index++;
		}
}







void Keypad_init_gpio(const uint16_t *row_pins, const uint16_t *col_pins)
{
	for(int row = 0; row < ROWS; row++)
  {
			for (int col = 0; col < COLS; col++)
      {
					keys_state[row][col].key_value = keys[row][col];
			}
	}
	
	    // Проверка на NULL для безопасности
    if (row_pins == 0 || col_pins == 0) return;
   
    GPIO_InitTypeDef gpio_init_out = {0};
    gpio_init_out.GPIO_Pin = row_pins[0] | row_pins[1] | row_pins[2] | row_pins[3];
    gpio_init_out.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init_out.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEYPAD_GPIO, &gpio_init_out);
	
		GPIO_InitTypeDef gpio_init_in = {0};
		gpio_init_in.GPIO_Pin = col_pins[0] | col_pins[1] | col_pins[2] | col_pins[3];
    gpio_init_in.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEYPAD_GPIO, &gpio_init_in);
}

uint8_t Check_keypad_pressed(const uint16_t *row_pins, const uint16_t *col_pins)
{
    static uint8_t last_key = NO_KEY;

    for(int row = 0; row < ROWS; row++)
    {
        GPIO_ResetBits(KEYPAD_GPIO, row_pins[0] | row_pins[1] | row_pins[2] | row_pins[3]);
        GPIO_SetBits(KEYPAD_GPIO, ~row_pins[row]);

        for(int col = 0; col < COLS; col++)
        {
            uint8_t current_phys_state = GPIO_ReadInputDataBit(KEYPAD_GPIO, col_pins[col]); // Физическое состояние пина
            
            // Если физически кнопка замкнута (LOW)
            if (current_phys_state == Bit_RESET)
            {
                // --- ЛОГИКА НАЖАТИЯ ---
                if (!keys_state[row][col].is_pressed)
                {
                    // Кнопка была отпущена, а теперь нажата. Запоминаем время.
                    keys_state[row][col].press_time = timer2_cur_time_ms;
                    keys_state[row][col].is_pressed = 1;
                    keys_state[row][col].key_value = keys[row][col];
                }
                else
                {
                    // Кнопка уже помечена как нажатая. Проверяем время дребезга.
                    if ((timer2_cur_time_ms - keys_state[row][col].press_time) >= KEYPAD_DEBOUNCE_TIME_MS)
                    {
                        // Дребезг прошел. Возвращаем код клавиши, если она еще не считана.
                        if (last_key != keys_state[row][col].key_value)
												{
                            last_key = keys_state[row][col].key_value;
														while (!GPIO_ReadInputDataBit(KEYPAD_GPIO, col_pins[col])) {}
                            return last_key;
                        }
                    }
                }
            }
            else
            {
                // --- ЛОГИКА ОТПУСКАНИЯ ---
                if (keys_state[row][col].is_pressed)
                {
                    // Кнопка была нажата, а теперь отпущена. Сбрасываем флаг.
                    keys_state[row][col].is_pressed = 0;
                    // Не сбрасываем last_key здесь! Это позволит функции вернуть код только один раз.
                }
            }
        }
    }

    last_key = NO_KEY;
    return NO_KEY;
}

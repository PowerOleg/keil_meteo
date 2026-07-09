#include "keypad4x4.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

uint8_t keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

volatile Key_state keys_state[ROWS][COLS];

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
/*	
    gpio_init_in.GPIO_Pin = col_pins[1] | col_pins[2] | col_pins[3];
    gpio_init_in.GPIO_Mode = GPIO_Mode_IPU;
//    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio_init_in);*/
}

uint8_t Check_keypad_pressed(const uint16_t *row_pins, const uint16_t *col_pins)
{
    static uint8_t last_key = NO_KEY; // Для отслеживания уже нажатой клавиши

    for(int row = 0; row < ROWS; row++)
    {
        // Активируем текущую строку (LOW)
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
                        if (last_key != keys_state[row][col].key_value) {
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

    // Если ни одна клавиша не готова к возврату, сбрасываем last_key
    last_key = NO_KEY;
    return NO_KEY;
}


static uint8_t Check_keypad_pressed1(
    const uint16_t *row_pins,
    const uint16_t *col_pins)
{
	static uint8_t last_key = NO_KEY;
    // 1. Проходим по всем клавишам
    for (int row = 0; row < ROWS; ++row)
    {
        // Активируем строку (остальные подтягиваются pull-up)
        GPIO_ResetBits(KEYPAD_GPIO, row_pins[row]);
        
        for (int col = 0; col < COLS; ++col)
        {
            // Читаем физическое состояние
            uint8_t pressed_now = !GPIO_ReadInputDataBit(KEYPAD_GPIO, col_pins[col]);
						
					volatile Key_state *state = &keys_state[row][col];//???
            // 2. Обработка нажатия
            if (pressed_now == Bit_RESET)
            {
                
                
                // Первый клик: начинаем отсчет дебоя
                if (!state->is_pressed)
                {
                    state->is_pressed = 1;
                    state->press_time = timer2_cur_time_ms;
                }
                else
                {
                    // Дебой пройден?
                    if ((timer2_cur_time_ms - state->press_time) >= KEYPAD_DEBOUNCE_TIME_MS)
                    {
                        // Уже отправили? (Анти-спам)
                        if (last_key != state->key_value)
                        {
                            last_key = state->key_value;
                            return last_key;
                        }
                    }
                }
            }
            // 3. Обработка отпускания
            else if (state->is_pressed)
            {
                // Сбрасываем только внутренний флаг
                state->is_pressed = 0;
            }
        }
    }

    // 4. Сброс "последней клавиши" делаем только здесь
    // Если мы дошли сюда, значит, активных клавиш нет
    last_key = NO_KEY;
    return NO_KEY;
}


#ifndef __KEYPAD4X4_H
#define __KEYPAD4X4_H

#include "stm32f10x.h"
#include "timer.h"

#define TIME_SIZE 5
#define DATE_SIZE 10

#define KEYPAD_DEBOUNCE_TIME_MS 40
#define NO_KEY 									0xFF // Константа для обозначения отсутствия нажатия
#define ROWS 										4 // Четыре строки
#define COLS 										4 // Четыре столбца
#define KEYPAD_GPIO 						GPIOB

// Создаем массив символов на кнопках
extern uint8_t keys[ROWS][COLS];
extern volatile uint8_t symbol_index;

extern const uint16_t row_pins[ROWS];
extern const uint16_t col_pins[COLS];

typedef struct {
    uint8_t is_pressed;   // Флаг: 1 - кнопка сейчас считается нажатой
    uint8_t key_value;    // Значение клавиши ('1', 'A' и т.д.)
    uint32_t press_time;  // Время последнего изменения состояния (нажатия или отпускания)
} Key_state;

// Создаем массив состояний для всех клавиш
extern volatile Key_state keys_state[ROWS][COLS];

void Keypad_init_gpio(void);
uint8_t Check_keypad_pressed(void);
char Keypad_listen(const uint16_t *row_pins, const uint16_t *col_pins);
void Input_time(uint8_t *time, const uint8_t pressed_key);
void Input_date(uint8_t *date, const uint8_t pressed_key);

#endif

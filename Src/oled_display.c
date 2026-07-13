#include "oled_display.h"
#include "stm32f10x_spi.h"
#include <stdio.h>
#include <string.h>

uint8_t oled_screen[OLED_BUFSIZE]; // 1024 байта

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
		slash_5x7,					// 36
		letter_H_5x7				// 37
};





const uint8_t colon_5x7[COL_PX] = {0x00, 0x36, 0x36, 0x00, 0x00};// Двоеточие
const uint8_t minus_5x7[COL_PX] = {0x00, 0x08, 0x08, 0x08, 0x00};// Минус
const uint8_t dot_5x7[COL_PX]   = {0x00, 0x60, 0x60, 0x00, 0x00};// Точка
const uint8_t degree_5x7[COL_PX]= {0x06, 0x09, 0x09, 0x06, 0x00};// Градус (°)
const uint8_t letter_C_5x7[COL_PX] = {0x3E, 0x41, 0x41, 0x41, 0x22};// Буква C
const uint8_t percent_5x7[COL_PX] = {0x11, 0x10, 0x0E, 0x01, 0x11};//{0x03, 0x19, 0x04, 0x13, 0x18};//{0x11, 0x10, 0x0E, 0x01, 0x11};//{0x61, 0x52, 0x0C, 0x52, 0x61};  // '%'
const uint8_t percent_15x21[15] = {0};  // '%'
const uint8_t exclamation_5x7[COL_PX] = {0x00, 0x5F, 0x00, 0x00, 0x00}; // !

//const uint8_t letter_r_5x7[5]   = {0x7F, 0x09, 0x19, 0x29, 0x46}; // р
const uint8_t letter_V_cap_5x7[5] = {0x7F, 0x49, 0x49, 0x49, 0x36};// B
const uint8_t letter_p_5x7[5] = {0x7F, 0x09, 0x09, 0x09, 0x06}; // p
const uint8_t letter_E_cap_5x7[5] = {0x7F, 0x49, 0x49, 0x49, 0x41}; // E
const uint8_t letter_m_5x7[5]   = {0x7F, 0x02, 0x0C, 0x02, 0x7F}; // м
const uint8_t letter_M_cap_5x7[5] = {0x7F, 0x02, 0x04, 0x02, 0x7F}; // M
const uint8_t letter_Ya_cap_5x7[5] = {0x46, 0x29, 0x19, 0x09, 0x7F}; //Я
// Пробел (пустые 5 столбцов)
const uint8_t space_5x7[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t letter_T_cyr_5x7[5] = {0x01, 0x01, 0x7F, 0x01, 0x01}; //T
const uint8_t letter_ru_P_cap_5x7[5] = {0x7F, 0x01, 0x01, 0x01, 0x7F}; //П
const uint8_t letter_A_cap_5x7[5] = {0x7E, 0x09, 0x09, 0x09, 0x7E};
const uint8_t hyphen_5x7[5] = {0x00, 0x08, 0x08, 0x08, 0x00}; // дефис
const uint8_t letter_L_cap_5x7[5] = {0x40, 0x3F, 0x01, 0x01, 0x7F};
const uint8_t letter_Zh_cap_5x7[5] = {0x63, 0x14, 0x7F, 0x14, 0x63};// Ж
const uint8_t letter_N_cap_5x7[5] = {0x7F, 0x08, 0x08, 0x08, 0x7F};// Н 
const uint8_t letter_O_cap_5x7[5] = {0x3E, 0x41, 0x41, 0x41, 0x3E};// О
const uint8_t letter_SoftSign_5x7[5] = {0x7F, 0x48, 0x48, 0x48, 0x30};// Ь
const uint8_t letter_D_cap_5x7[5] = {0x60, 0x3E, 0x21, 0x3F, 0x60};// Д
const uint8_t letter_I_cap_5x7[5] = {0x7F, 0x20, 0x10, 0x08, 0x7F};// И 
const uint8_t letter_U_cap_5x7[5] = {0x03, 0x44, 0x48, 0x48, 0x3F};//У
const uint8_t slash_5x7[5] = {0x20, 0x10, 0x08, 0x04, 0x02};// /
const uint8_t letter_H_5x7[5] = {0x7F, 0x08, 0x08, 0x08, 0x7F};// H

// Индексы: В(20), Р(18), Е(21), M(22), Я(23), :(10)
const uint8_t vremya_indices[] = {20, 18, 21, 22, 23, 10};
// Индексы: Т(19), Е(21), M(22), П(24), -(26), Р(18), А(25), :(10)
const uint8_t temperature_indices[] = {19, 21, 22, 24, 26, 18, 25, 10};
// Индексы: Т(19), Е(21), M(22), П(24), Е(21), Р(18), А(25), Т(19), У(34), Р(18), А(25):(10)
const uint8_t temperature_full_indices[] = {19, 21, 22, 24, 21, 18, 25, 19, 34, 18, 25, 10};
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



//Битовые маски символов (5x7)
// Цифры 0..9
const uint8_t digit_font_5x7[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}  // 9
};

void Oled_gpio_init(void)
{
	// Управляющие пины OLED (PA9, PA10, PA11)
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // Начальные уровни
    OLED_CS_HIGH();
    OLED_DC_COMMAND();
    OLED_RES_HIGH();
}

void OLED_Reset(void)
{
    OLED_RES_LOW();
    Delay_us(10000);
    OLED_RES_HIGH();
    Delay_us(10000);
}

void OLED_Init_SSD1306(void)
{
    OLED_Reset(); // аппаратный сброс
    OLED_WriteByte(0xAE, 0); // Выключить дисплей
    OLED_WriteByte(0xD5, 0); // Установить делитель тактовой частоты
    OLED_WriteByte(0x80, 0); // Стандартное значение
    OLED_WriteByte(0xA8, 0); // Мультиплексирование (высота)
    OLED_WriteByte(0x3F, 0); // 64 строки (0x3F = 63d, т.е. 64 строки)
    OLED_WriteByte(0xD3, 0); // Сдвиг отображения
    OLED_WriteByte(0x00, 0); // Без сдвига
    OLED_WriteByte(0x40, 0); // Начальная строка отображения (0x40 – обычно 0)
    OLED_WriteByte(0x8D, 0); // Накачка заряда (Charge Pump)
    OLED_WriteByte(0x14, 0); // Включить (если внешнее питание, 0x10 отключить)
    OLED_WriteByte(0x20, 0); // Режим адресации памяти
    OLED_WriteByte(0x00, 0); // Горизонтальный режим (или 0x02 – страничный, для простоты возьмём страничный 0x02)
    OLED_WriteByte(0xA1, 0); // Направление сегментов (0xA0 или 0xA1 – зеркалирование)
    OLED_WriteByte(0xC8, 0); // Направление COM-выходов (0xC0 или 0xC8 – переворот)
    OLED_WriteByte(0xDA, 0); // Конфигурация выводов COM
    OLED_WriteByte(0x12, 0); // Для 128x64
    OLED_WriteByte(0x81, 0); // Контраст
    OLED_WriteByte(0xCF, 0); // Значение по умолчанию
    OLED_WriteByte(0xD9, 0); // Предзаряд
    OLED_WriteByte(0xF1, 0); 
    OLED_WriteByte(0xDB, 0); // VCOMH
    OLED_WriteByte(0x40, 0);
    OLED_WriteByte(0xA4, 0); // Режим "все пиксели выкл" (0xA4 – нормальный, 0xA5 – все вкл)
    OLED_WriteByte(0xA6, 0); // Не инвертировать (0xA6 – нормальный, 0xA7 – инверсный)
    OLED_WriteByte(0x2E, 0); // Остановить скроллинг
    OLED_WriteByte(0xAF, 0); // Включить дисплей

}

static uint8_t Get_dozens(const uint8_t number)
{
		uint8_t dozens = 0;
		if (number > 9)
		{
				dozens = number / 10 % 10;
		}
		
		return dozens;
}

void OLED_WriteByte(uint8_t data, uint8_t dc)
{
    if (dc)
        OLED_DC_DATA();  // данные
    else
        OLED_DC_COMMAND();   // команда
    
    OLED_CS_LOW();
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
		
		//очищаем от застривающего в буфере байта
		if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == SET) {
        volatile uint8_t dummy = SPI_I2S_ReceiveData(SPI1);
        (void)dummy;
    }
    OLED_CS_HIGH();
}

void OLED_ClearBuffer(void)
{
    memset(oled_screen, 0x00, OLED_BUFSIZE);
}

void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    uint16_t index = x + (y / 8) * OLED_WIDTH;
    if (color)
        oled_screen[index] |= (1 << (y & 7));
    else
        oled_screen[index] &= ~(1 << (y & 7));
}

void OLED_UpdateScreen(void)
{
    // Установить диапазон столбцов
    OLED_WriteByte(0x21, 0); OLED_WriteByte(0, 0); OLED_WriteByte(127, 0);
    // Установить диапазон страниц
    OLED_WriteByte(0x22, 0); OLED_WriteByte(0, 0); OLED_WriteByte(7, 0);

    // Пересылка данных
    for (uint16_t i = 0; i < OLED_BUFSIZE; i++) {
        OLED_WriteByte(oled_screen[i], 1);
    }
}

void OLED_DrawScaledChar(uint8_t x0, uint8_t y0, const uint8_t *bitmap, uint8_t scale)
{
    uint8_t col, row, bit;
    for (col = 0; col < COL_PX; col++) {               // 5 столбцов символа
        for (row = 0; row < ROW_PX; row++) {           // 7 строк символа
            if (bitmap[col] & (1 << row)) {       // если пиксель активен
                // рисуем квадрат scale x scale
                for (uint8_t dx = 0; dx < scale; dx++) {
                    for (uint8_t dy = 0; dy < scale; dy++) {
                        OLED_SetPixel(x0 + col * scale + dx, y0 + row * scale + dy, 1);
                    }
                }
            }
        }
    }
}

// Установить текущую страницу (0..7) и колонку (0..127)
void OLED_SetPos(uint8_t page, uint8_t col)
{
    OLED_WriteByte(0xB0 | (page & 0x07), 0);   // выбор страницы
    OLED_WriteByte(0x00 | (col & 0x0F), 0);    // младшая часть колонки
    OLED_WriteByte(0x10 | ((col >> 4) & 0x0F), 0); // старшая часть
}

void OLED_DrawPercent15x21(uint8_t x, uint8_t y)
{
	 // Страница 0 (строки 0..7)
    OLED_SetPos(y, x);
    const uint8_t page0[15] = {
        0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x10
    };
    for (int i = 0; i < 15; i++) OLED_WriteByte(page0[i], 1);

    // Страница 1 (строки 8..15)
    OLED_SetPos(y + 1, x);
    const uint8_t page1[15] = {
        0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00
    };
    for (int i = 0; i < 15; i++) OLED_WriteByte(page1[i], 1);

    // Страница 2 (строки 16..23)
    OLED_SetPos(y + 2, x);
    const uint8_t page2[15] = {
        0x08, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70
    };
    for (int i = 0; i < 15; i++) OLED_WriteByte(page2[i], 1);
	}

	
	// Рисует знак процента 15?21 пиксель в глобальный буфер oled_screen
// x_px   – координата левого верхнего угла по горизонтали (0..127)
// y_page – координата страницы (0..7), начало знака по вертикали
void OLED_DrawPercent15x21Buf(uint8_t x_px, uint8_t y_page)
{
    // Массивы для трёх страниц (точно как в OLED_DrawPercent15x21)
    const uint8_t page0[15] = {
        0x0E, 0x11, 0x11, 0x11, 0x0E,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x40, 0x20, 0x10
    };
    const uint8_t page1[15] = {
        0x00, 0x00, 0x00, 0x00, 0x40,
        0x20, 0x10, 0x08, 0x04, 0x02,
        0x01, 0x00, 0x00, 0x00, 0x00
    };
    const uint8_t page2[15] = {
        0x08, 0x04, 0x02, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x70, 0x88, 0x88, 0x88, 0x70
    };

    // Базовое смещение по Y в пикселях
    uint8_t y_base = y_page * 8;

    // Обрабатываем три страницы (0–2)
    const uint8_t* pages[3] = {page0, page1, page2};
    for (uint8_t p = 0; p < 3; p++) {
        uint8_t y_offset = p * 8;               // смещение внутри знака (0,8,16)
        for (uint8_t col = 0; col < 15; col++) {
            uint8_t byte = pages[p][col];
            if (byte == 0) continue;
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (byte & (1 << bit)) {
                    uint8_t py = y_base + y_offset + bit; // реальная Y координата
                    // Знак занимает строки 0..20, игнорируем выход за пределы
                    if (py < OLED_HEIGHT) {
                        OLED_SetPixel(x_px + col, py, 1);
                    }
                }
            }
        }
    }
}
	
	
// Печать строки, состоящей из символов с известными масками.
// symbol_table – массив указателей на 5-байтовые маски,
// str – индексы в таблице (например, для цифр 0..9 индекс = сама цифра,
// для двоеточия – 10, для букв – 11..15 и т.д.)
    // y=30 даёт отступ от надписи (высота 2*7=14 + 2=16, значит от 30 норм)
void OLED_PrintScaledSymbols(uint8_t x0, uint8_t y0, const uint8_t **symbol_table,
                            const uint8_t *symbols, uint8_t count, uint8_t scale)
{
    uint8_t x = x0;
    for (uint8_t i = 0; i < count; i++)
		{
        uint8_t idx = symbols[i];
				OLED_DrawScaledChar(x, y0, symbol_table[idx], scale);
        x += 5 * scale + scale; // ширина символа + отступ в 1 пиксель (scale)
        if (x > OLED_WIDTH - 5 * scale) break; // защита от выхода за границу
    }
}

void OLED_fill_indices(uint8_t *indices, const uint8_t hours, const uint8_t minutes)
{
		uint8_t dozens_hours = Get_dozens(hours);
		indices[0] = dozens_hours;
		indices[1] = hours % 10;
		uint8_t dozens_minutes = Get_dozens(minutes);
		indices[3] = dozens_minutes;
		indices[4] = minutes % 10;
}

void OLED_PrintTemperature(uint8_t x, uint8_t y, float temperature, uint8_t scale, const uint8_t **font_table)
{
		uint8_t indices[10];  // достаточно для -XX.X°C
    uint8_t cnt = 0;

    // Знак
    if (temperature < 0) {
        indices[cnt++] = 11;  // минус
        temperature = -temperature;//цифра отрицательная но чтобы работало, надо превратить в положительную
    }

    // Целая часть
    int whole = (int)temperature;
    uint8_t whole_digits[3];
    uint8_t whole_len = 0;
    if (whole == 0)
		{
        whole_digits[0] = 0;
        whole_len = 1;
    }
		else
		{
        while (whole > 0)
				{
            whole_digits[whole_len++] = whole % 10;
            whole /= 10;
				}
				
        // Развернуть порядок (старший разряд первый)
        for (uint8_t i = 0; i < whole_len / 2; i++)
				{
            uint8_t t = whole_digits[i];
            whole_digits[i] = whole_digits[whole_len - 1 - i];
            whole_digits[whole_len - 1 - i] = t;
        }
    }
    for (uint8_t i = 0; i < whole_len; i++)
		{
        indices[cnt++] = whole_digits[i]; // цифра 0..9
    }

    // Десятая доля (одна цифра после запятой)
    indices[cnt++] = 12; // точка
    int frac = (int)(temperature * 10) % 10; // округление до ближайшего целого уже в temperature
    // Более точное округление:
    // int frac = (int)((temperature - whole) * 10 + 0.5);
    indices[cnt++] = frac; // цифра 0..9

    // Градус и C
    indices[cnt++] = 13;   // °
    indices[cnt++] = 14;   // C

    // Вывод строки
    OLED_PrintScaledSymbols(x, y, font_table, indices, cnt, scale);
}


void OLED_PrintHumidity(uint8_t x, uint8_t y, float humidity, uint8_t scale, const uint8_t **font_table)
{
		uint8_t indices[6];   // максимум: две цифры, точка, десятая, '%' => 5 символов
    uint8_t cnt = 0;

    // Ограничиваем диапазон 0.0 – 100.0
    if (humidity < 0.0f) humidity = 0.0f;
    if (humidity > 100.0f) humidity = 100.0f;

    // Целая часть
    int whole = (int)humidity;
    uint8_t whole_digits[3];
    uint8_t whole_len = 0;
    if (whole == 0) {
        whole_digits[0] = 0;
        whole_len = 1;
    } else {
        while (whole > 0) {
            whole_digits[whole_len++] = whole % 10;
            whole /= 10;
        }
        // Разворачиваем (старший разряд первым)
        for (uint8_t i = 0; i < whole_len / 2; i++) {
            uint8_t tmp = whole_digits[i];
            whole_digits[i] = whole_digits[whole_len - 1 - i];
            whole_digits[whole_len - 1 - i] = tmp;
        }
    }
    for (uint8_t i = 0; i < whole_len; i++) {
        indices[cnt++] = whole_digits[i];   // 0..9
    }

    // Десятая доля
    indices[cnt++] = 12;  // точка (.)
    int frac = (int)((humidity - whole) * 10 + 0.5f);  // округление
    if (frac > 9) frac = 9;
    indices[cnt++] = frac;  // 0..9

    
//    indices[cnt++] = 15;   // '%'

    // Вывод строки
    OLED_PrintScaledSymbols(x, y, font_table, indices, cnt, scale);
}

void OLED_PrintPressure(/*uint8_t x, */uint8_t y, uint32_t pressure, uint8_t scale, const uint8_t **font_table)
{
    // --- Преобразование числа в индексы цифр ---
    uint8_t digits[10];
    uint8_t len = 0;
    if (pressure == 0) {
        digits[len++] = 0;
    } else {
        uint32_t temp = pressure;
        while (temp > 0) {
            digits[len++] = temp % 10;
            temp /= 10;
        }
        // разворот (старший разряд первым)
        for (uint8_t i = 0; i < len / 2; i++) {
            uint8_t t = digits[i];
            digits[i] = digits[len - 1 - i];
            digits[len - 1 - i] = t;
        }
    }

    // --- Центрирование числа ---
    // ширина одного символа = 5*scale, зазор между символами = scale
    // общая ширина без конечного зазора = len*5*scale + (len-1)*scale
    uint16_t num_width = (len * 6 - 1) * scale;
    uint8_t x_num = (OLED_WIDTH - num_width) / 2;
    if (x_num > OLED_WIDTH) x_num = 0;
    OLED_PrintScaledSymbols(x_num, y, font_table, digits, len, scale);

    // --- Единица измерения «мм.рт.ст.» (масштаб 2) ---
    const uint8_t unit_idx[] = {17, 17, 12, 18, 19, 12, 14, 19};
    //                          м   м   .    р   т   .    с   т   .
    const uint8_t unit_len = sizeof(unit_idx);
    const uint8_t unit_scale = 2;
    uint16_t unit_width = (unit_len * 6 - 1) * unit_scale;
    uint8_t x_unit = (OLED_WIDTH - unit_width) / 2;
    if (x_unit > OLED_WIDTH) x_unit = 0;

    // Отступ вниз от цифр (высота цифр = 7*scale + небольшой зазор)
    uint8_t y_unit = y + 7 * scale + 2;
    if (y_unit > OLED_HEIGHT - 7*unit_scale) y_unit = OLED_HEIGHT - 7*unit_scale;

    OLED_PrintScaledSymbols(x_unit, y_unit, font_table, unit_idx, unit_len, unit_scale);
}

//110726 flash

void Display_flash_data(char *flash_buff, const uint8_t current_page_number, const uint8_t display_number)// 39 символов + null terminator
{
		//t 30 [2026-07-02 08:01:03]
	
		const uint8_t page_offset = 20;
		uint8_t line1_size = 20;
		uint8_t line2_size = 20;
    static uint8_t oled_line1[20];
		static uint8_t oled_line2[20];
//		char oled_symbols_test[40];
		


//		char* found = strchr(flash_buff, '[');
//		uint8_t index_date_start = found - flash_buff;
	
		
    // Копируем строку в массив символов
//    memcpy(oled_symbols, flash_buff, strlen(flash_buff));

    // Отображаем номер страницы
		// Индексы: В(20), Л(27), А(25), Ж(28), Н(29), О(30), С(14), Т(19), Ь(31), :(10)
//const uint8_t humidity_indices[] = {20, 27, 25, 28, 29, 30, 14, 19, 31, 10};
//    OLED_PrintScaledSymbols(0, 0, font_table, (const uint8_t*)"Страница ", 9, 2);
//1. 
/*		uint8_t page_num[1];
		page_num[0] = current_page_number;//(const uint8_t*)
    OLED_PrintScaledSymbols(86, 0, font_table, page_num, 1, 1);*/
		
		
		uint8_t index_count = 0;//индекс чтобы понять какой символ отображаем в строке oled дисплея
		oled_line1[index_count++] = current_page_number;
		oled_line1[index_count++] = 12;
//		oled_line1[index_count++] = 35;

		int i = 0;//индекс чтобы понять какой символ берем из массива с данными из flash 
		if (flash_buff[i] == 'T' || flash_buff[i] == 't')
		{
				memcpy(oled_line1 + 2, temperature_full_indices, 11);
				index_count += 11;
				//oled_line1[index_count++] = 19;
		}
		else if (flash_buff[i] == 'P' || flash_buff[i] == 'p')
		{		
				memcpy(oled_line1 + 2, pressure_indices, 8);
				index_count += 8;
				//oled_line1[index_count++] = 18;
		}
		else if (flash_buff[i] == 'H' || flash_buff[i] == 'h')
		{
				memcpy(oled_line1 + 2, humidity_indices, 9);
				index_count += 9;
				//oled_line1[index_count++] = 37;
		}
		else
				return;

		
		oled_line1[index_count++] = 35;
		i += 2;
		
				for (; i < line1_size; i++)
				{
						if (flash_buff[i] == '[')
						{
								line1_size = index_count;
								break;//continue;
						}
						else if (flash_buff[i] == ' ')
								oled_line1[index_count++] = 35;
						else if (flash_buff[i] == ']')
						{
								line1_size = i;
								break;
						}
						else if (flash_buff[i] == '\n')
						{
								continue;//line1_size = index_count;
								//break;
						}
						else if (flash_buff[i] == ':')
								oled_line1[index_count++] = 10;
						else if (flash_buff[i] == '-')
								oled_line1[index_count++] = 11;
						else 
								oled_line1[index_count++] = flash_buff[i] - '0';
				}
				int j = 0;
				for (; j < line2_size; i++)
				{
						if (flash_buff[i] == '[')
								continue;
						else if (flash_buff[i] == ' ')
								oled_line2[j++] = 35;
						else if (flash_buff[i] == ']')
						{
								line2_size = j;
								break;
						}
						else if (flash_buff[i] == '\n')
								oled_line2[j++] = 35;
						else if (flash_buff[i] == ':')
								oled_line2[j++] = 10;
						else if (flash_buff[i] == '-')
								oled_line2[j++] = 11;
						else
								oled_line2[j++] = flash_buff[i] - '0';
				}
//				line2_size = j;
		
		
//		oled_line1[(line1_size/* + line2_size*/)] = '\0';
    // Отображаем содержимое Flash-памяти
    OLED_PrintScaledSymbols(0, ((display_number - 1) * page_offset), font_table, oled_line1, line1_size, 1);
		OLED_PrintScaledSymbols(0, (((display_number - 1) * page_offset) + 10), font_table, oled_line2, line2_size, 1);
}

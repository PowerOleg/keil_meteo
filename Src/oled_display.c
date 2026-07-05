#include "oled_display.h"
#include "stm32f10x_spi.h"
#include <stdio.h>
#include <string.h>

uint8_t oled_screen[OLED_BUFSIZE]; // 1024 байта

const uint8_t colon_5x7[COL_PX] = {0x00, 0x36, 0x36, 0x00, 0x00};// Двоеточие
const uint8_t minus_5x7[COL_PX] = {0x00, 0x08, 0x08, 0x08, 0x00};// Минус
const uint8_t dot_5x7[COL_PX]   = {0x00, 0x60, 0x60, 0x00, 0x00};// Точка
const uint8_t degree_5x7[COL_PX]= {0x06, 0x09, 0x09, 0x06, 0x00};// Градус (°)
const uint8_t letter_C_5x7[COL_PX] = {0x3E, 0x41, 0x41, 0x41, 0x22};// Буква C
const uint8_t percent_5x7[COL_PX] = {0x11, 0x10, 0x0E, 0x01, 0x11};//{0x03, 0x19, 0x04, 0x13, 0x18};//{0x11, 0x10, 0x0E, 0x01, 0x11};//{0x61, 0x52, 0x0C, 0x52, 0x61};  // '%'
const uint8_t percent_15x21[15] = {0};  // '%'
const uint8_t exclamation_5x7[COL_PX] = {0x00, 0x5F, 0x00, 0x00, 0x00}; // !

//const uint8_t letter_r_5x7[5]   = {0x7F, 0x09, 0x19, 0x29, 0x46}; // р
const uint8_t letter_p_5x7[5] = {0x7F, 0x09, 0x09, 0x09, 0x06}; // p
const uint8_t letter_m_5x7[5]   = {0x7F, 0x02, 0x0C, 0x02, 0x7F}; // м
// Пробел (пустые 5 столбцов)
const uint8_t space_5x7[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
// Буква 'т' (строчная, как T)
const uint8_t letter_T_cyr_5x7[5] = {0x01, 0x01, 0x7F, 0x01, 0x01};

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



void Oled_spi_init(void)
{
    SPI_InitTypeDef SPI_InitStructure;
    
    // Отключаем SPI перед настройкой
    SPI_Cmd(SPI1, DISABLE);
    
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx; // Только передача, MISO не используется
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;   // SSD1306: CPOL=1, CPHA=1 (режим 3)
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;     // Программное управление CS
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // Тактовая SPI ? 10 МГц (APB2=72МГц > 9 МГц)
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    
    SPI_Cmd(SPI1, ENABLE);
}

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
	
	
    /*GPIO_InitTypeDef GPIO_InitStructure;
    
    // Включаем тактирование портов и SPI1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1, ENABLE);
    
    // SCK (PA5) и MOSI (PA7) — альтернативная функция push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // CS (PA11), DC (PA9), RES (PA10)//было CS (PC14), DC (PA0), RES (PC15) — выходы push-pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;  //CS (PA11), DC (PA9), RES (PA10)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // Начальные уровни: CS высокий (не активен), DC низкий, RES высокий
    OLED_CS_HIGH();//GPIO_SetBits(GPIOA, GPIO_Pin_4);   // CS = 1
    OLED_DC_COMMAND();//GPIO_ResetBits(GPIOB, GPIO_Pin_1); // DC = 0
    OLED_RES_HIGH();   // RES = 1*/
}

/*
void Oled_send_data(uint8_t data)
{
    OLED_DC_DATA(); // Устанавливаем линию DC в состояние DATA
    OLED_CS_LOW(); // Опускаем CS
    SPI_I2S_SendData(SPI1, data);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    OLED_CS_HIGH(); // Обязательно поднимаем CS обратно!
}

// Отправляет команду
void Oled_send_command(uint8_t command)
{
    OLED_DC_COMMAND(); // Устанавливаем линию DC в состояние COMMAND
    OLED_CS_LOW(); // Опускаем CS
    SPI_I2S_SendData(SPI1, command);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    OLED_CS_HIGH(); // Поднимаем CS
		Delay_us(1000);
}*/


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

/*

// Залить экран байтом (0x00 – очистить, 0xFF – полностью зажечь)
void OLED_Fill(uint8_t data)
{
    for (uint8_t page = 0; page < 8; page++) {
        OLED_SetPos(page, 0);
        for (uint16_t col = 0; col < 128; col++) {
            OLED_WriteByte(data, 1); // посылаем данные
        }
    }
}
void OLED_Clear(void)
{
    OLED_Fill(0x00);
}
void GetCharBitmap(char c, uint8_t *bitmap)
{
    // Заполняем bitmap пятью байтами
    switch(c) {
        case 'H': memcpy(bitmap, (uint8_t[]){0x7F,0x08,0x08,0x08,0x7F}, 5); break;
        case 'e': memcpy(bitmap, (uint8_t[]){0x38,0x54,0x54,0x54,0x18}, 5); break;
        case 'l': memcpy(bitmap, (uint8_t[]){0x00,0x41,0x7F,0x40,0x00}, 5); break;
        case 'o': memcpy(bitmap, (uint8_t[]){0x38,0x44,0x44,0x44,0x38}, 5); break;
        default:  memset(bitmap, 0, 5); break; // пробел или неизвестный
    }
}

// Вывод одного символа в позиции (page, col)
void OLED_DrawChar(uint8_t page, uint8_t col, char c)
{
    uint8_t bitmap[5];
    GetCharBitmap(c, bitmap);
    OLED_SetPos(page, col);
    for (uint8_t i = 0; i < 5; i++) {
        OLED_WriteByte(bitmap[i], 1); // данные
    }
    // Пустой столбец-разделитель
    OLED_WriteByte(0x00, 1);
}

// Печать строки. Перенос на следующую страницу при выходе за границу.
void OLED_PrintString(uint8_t page, uint8_t col, const char *str)
{
    while (*str)
		{
        if (col > 127 - 5) {   // не влезает
            col = 0;
            page++;
            if (page > 7) return; // за пределом экрана
        }
        OLED_DrawChar(page, col, *str);
        col += 6;              // 5 пикс. символа + 1 пробел
        str++;
    }
}*/


//functions for working with the display buffer

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
    const uint8_t page0[15] = {								// кружок (строки 1-5 cols 0-4)
        0x0E, 0x11, 0x11, 0x11, 0x0E,         0x00, 0x00, 0x00, 0x00, 0x00,         0x00, 0x00, 0x40, 0x20, 0x10 // col5: бит 5 (строка 5) – начало линии
    };
    for (int i = 0; i < 15; i++) OLED_WriteByte(page0[i], 1);

    // Страница 1 (строки 8..15)
    OLED_SetPos(y + 1, x);
    const uint8_t page1[15] = {						// col6: бит6, col7: бит5, col8: бит4, col9: бит3 (диагональ)
        0x00, 0x00, 0x00, 0x00, 0x40,        0x20, 0x10, 0x08, 0x04, 0x02,        0x01, 0x00, 0x00, 0x00, 0x00  // col10: бит2 (строка 10)
    };
    for (int i = 0; i < 15; i++) OLED_WriteByte(page1[i], 1);

    // Страница 2 (строки 16..23)
    OLED_SetPos(y + 2, x);
    const uint8_t page2[15] = {								// col9: бит1 (строка 17)
        0x08, 0x04, 0x02, 0x01, 0x00,        0x00, 0x00, 0x00, 0x00, 0x00,        0x70, 0x88, 0x88, 0x88, 0x70  // кружок cols 10-14: строки 16-20, плюс бит0 col10=1 для соединения
    };
    for (int i = 0; i < 15; i++) OLED_WriteByte(page2[i], 1);
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
        temperature = -temperature;
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

				/*letter_m_5x7,				// 17
		letter_r_5x7,				// 18
		letter_T_cyr_5x7,		// 19
		letter_C_5x7				// 20*/
    // --- Единица измерения «мм.рт.ст.» (масштаб 2) ---
    const uint8_t unit_idx[] = {17, 17, 12, 18, 19, 12, 20, 19};
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

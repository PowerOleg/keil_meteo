#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "common.h"
#include "lcd1602.h"
#include "stm32f10x_i2c.h"

char lcd_buffer[LCD_LINE_SIZE] = {' '};


void Lcd_i2c_send_byte(I2C_TypeDef* i2cx, uint8_t address, uint8_t byte)
{
    // Генерация условия СТАРТ
    I2C_GenerateSTART(i2cx, ENABLE);
    
    // Ожидание события СТАРТ
    while(!I2C_CheckEvent(i2cx, I2C_EVENT_MASTER_MODE_SELECT));
    
    // Отправка адреса устройства
    I2C_Send7bitAddress(i2cx, address, I2C_Direction_Transmitter);
    
    // Ожидание подтверждения выбора режима передачи
    while(!I2C_CheckEvent(i2cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    
    // Ожидание освобождения регистра передачи
    while(!(i2cx->SR1 & I2C_SR1_TXE));
    
    // Отправка байта данных
    I2C_SendData(i2cx, byte);
    
    // Ожидание окончания передачи байта
    while(!I2C_CheckEvent(i2cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));//(!(i2cx->SR1 & I2C_SR1_BTF));I2C_EVENT_MASTER_BYTE_TRANSMITTED
    
    // Генерация условия СТОП
    I2C_GenerateSTOP(i2cx, ENABLE);
}

//without dma
void Lcd_send_nibble(uint8_t nibble, uint8_t rs)
{
		uint8_t data = (nibble & 0x0F) << 4; // старшие 4 бита с моими данными (D4-D7)
    data |= (rs ? LCD_RS : 0);
    data |= LCD_BACKLIGHT;            // подсветка всегда включена
    // RW = 0, EN = 0
		
		Lcd_i2c_send_byte(I2C1, LCD_I2C_ADDRESS, data);
		data |= LCD_ENABLE;
		Lcd_i2c_send_byte(I2C1, LCD_I2C_ADDRESS, data);
		data &= ~LCD_ENABLE;
		Lcd_i2c_send_byte(I2C1, LCD_I2C_ADDRESS, data);
		Delay_us(50);
}

void Lcd_send_byte(uint8_t byte, uint8_t rs)
{
    Lcd_send_nibble((byte >> 4) & 0x0F, rs);
    Lcd_send_nibble(byte & 0x0F, rs);
}

void Lcd_send_command(uint8_t cmd)
{
    Lcd_send_byte(cmd, 0);
}

void Lcd_send_data(uint8_t data)
{
    Lcd_send_byte(data, 1);
}


void Lcd_init(void)
{		
		Delay_us(100);
    Lcd_send_nibble(0x03, 0);
    Delay_us(50);
    Lcd_send_nibble(0x03, 0);
    Delay_us(50);
    Lcd_send_nibble(0x03, 0);
    Delay_us(50);
    Lcd_send_nibble(0x02, 0);
    Delay_us(50);
	
    Lcd_send_command(LCD_FUNCTION_SET);// 4 бит, 2 строки, 5x8
    Lcd_send_command(LCD_DISPLAY_ON);
    Lcd_send_command(LCD_CLEAR_DISPLAY);
    Delay_us(10);
    Lcd_send_command(LCD_ENTRY_MODE);// инкремент адреса, без сдвига
		Delay_us(50);
}

void Lcd_print(const uint8_t* str, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++) 
		{
        Lcd_send_data(str[i]);
    }
}

void Lcd_set_cursor_position(uint8_t col, uint8_t row)
{
    static const uint8_t row_offsets[] = {0x00, 0x40};
    if (row > 1) row = 1;
    Lcd_send_command(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]));
}

void Lcd_clear(void)
{
    Lcd_send_command(LCD_CLEAR_DISPLAY);
}

/*
//for DMA
void Lcd_send_nibble(uint8_t nibble, uint8_t rs)
{
		uint8_t data = (nibble & 0x0F) << 4; // старшие 4 бита с моими данными (D4-D7)
    data |= (rs ? LCD_RS : 0);
    data |= LCD_BACKLIGHT;            // подсветка всегда включена
    // RW = 0, EN = 0

		// Подготовим три шага передачи данных через DMA
    uint8_t data_arr[3];
    data_arr[0] = data;              // Шаг 1: EN = 0
    data_arr[1] = data | LCD_ENABLE; // Шаг 2: EN = 1
    data_arr[2] = data;              // Шаг 3: EN = 0

    // Передадим все три шага через одну операцию DMA
    I2C_DMA_start(data_arr, 3);//I2C_send_data_by_DMA(data_arr, 3);
		Delay_us(5);
}*/

/*
void I2C_dma_init(void)
{
    // Включаем тактирование DMA
    RST_CLK_PCLKcmd(RST_CLK_PCLK_DMA, ENABLE);

    // Создаем структуру для первичной инициализации канала DMA
    DMA_ChannelInitTypeDef DMA_InitStruct;
    DMA_StructInit(&DMA_InitStruct);

    // Настраиваем параметры канала DMA
    DMA_InitStruct.DMA_PriCtrlData = 0;  // Первичная структура данных
    DMA_InitStruct.DMA_AltCtrlData = 0;  // Вторичная структура данных
    DMA_InitStruct.DMA_ProtCtrl = 0;        // Контроль защиты AHB-Lite
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;  // Высокий приоритет
    DMA_InitStruct.DMA_UseBurst = DMA_BurstClear;    // Отключаем пакетный режим
    DMA_InitStruct.DMA_SelectDataStructure = DMA_CTRL_DATA_PRIMARY;  // Используем основную структуру данных

    // Инициализируем канал DMA
    DMA_Init(DMA_Channel_SW1, &DMA_InitStruct);
}*/
/*void print_data(DHT22_Data data)
{
// Отображаем данные (например, через USART или LCD)
        printf("Temperature: %.1f C\nHumidity: %.1f %%\n",
               (float)(data.temperature)/10.0,
               (float)(data.humidity_integer)*10 + data.humidity_decimal);
}*/


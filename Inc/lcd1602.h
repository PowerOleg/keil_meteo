#ifndef __LCD1602_H
#define __LCD1602_H

#include "stm32f10x.h"



#define LCD_I2C_ADDRESS    (0x27 << 1)   // Адрес PCF8574
#define LCD_BACKLIGHT      0x08           // Бит подсветки
#define LCD_ENABLE         0x04           // Бит Enable
#define LCD_RW             0x02           // Бит Read/Write
#define LCD_RS             0x01           // Бит Register Select

#define LCD_CLEAR_DISPLAY  0x01
#define LCD_RETURN_HOME    0x02
#define LCD_ENTRY_MODE     0x06
#define LCD_DISPLAY_ON     0x0C
#define LCD_FUNCTION_SET   0x28
#define LCD_SET_DDRAM_ADDR 0x80

#define LCD_LINE_SIZE     16


extern char lcd_buffer[];

void Lcd_send_nibble(uint8_t nibble, uint8_t rs);
void Lcd_send_byte(uint8_t byte, uint8_t rs);
void Lcd_send_command(uint8_t cmd);
void Lcd_send_data(uint8_t data);

void Lcd_init(void);
void Lcd_print(const uint8_t* str, uint8_t length);
void Lcd_set_cursor_position(uint8_t col, uint8_t row);
void Lcd_clear(void);

void Lcd_i2c_send_byte(I2C_TypeDef* i2cx, uint8_t address, uint8_t byte);
//void I2C_send_data_by_DMA(uint8_t* buffer, uint16_t size);


//void print_dht22_data(DHT22_Data data);
#endif

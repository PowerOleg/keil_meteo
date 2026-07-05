#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "common.h"
#include "bme280.h"
#include "stm32f10x_spi.h"
#include "misc.h"
#include "timer.h"

#define CS_LOW()    GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define CS_HIGH()   GPIO_SetBits(GPIOA, GPIO_Pin_4)

// Глобальные переменные для калибровочных коэффициентов
//temperature
volatile uint16_t dig_T1;
volatile int16_t  dig_T2, dig_T3;
//pressure
volatile uint16_t dig_P1;
volatile int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
//humidity
volatile uint8_t  dig_H1;
volatile int16_t  dig_H2;
volatile uint8_t  dig_H3;
volatile int16_t  dig_H4;
volatile int16_t  dig_H5;
volatile int8_t   dig_H6;


void BME280_gpio_init(void)
{
    // Управляющий пин CSB (PA4)
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_4;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // Начальный уровень
    GPIO_SetBits(GPIOA, GPIO_Pin_4);  // CS = 1 (неактивен)
}


// Отправить/принять байт по SPI
uint8_t SPI_Transfer(uint8_t data)
{
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

// Чтение одного байта из регистра
uint8_t BME280_ReadByte(uint8_t reg)
{
    uint8_t val;
    CS_LOW();
    SPI_Transfer(reg | 0x80);   // установка бита 7 – операция чтения
    val = SPI_Transfer(0xFF);   // фиктивный байт для выталкивания ответа
    CS_HIGH();
    return val;
}

// Запись одного байта в регистр (используется редко, у вас была Write2Bytes)
// Для универсальности оставим однобайтную запись
void BME280_WriteByteInReg(uint8_t reg, uint8_t data)
{
    CS_LOW();
    SPI_Transfer(reg & 0x7F);   // сброс бита 7 – операция записи
    SPI_Transfer(data);
    CS_HIGH();
}

// Чтение нескольких байт подряд (начиная с reg, автоинкремент)
void BME280_ReadBytes(uint8_t reg, uint8_t *data, uint8_t len)
{
    CS_LOW();
    SPI_Transfer(reg | 0x80);
    for (uint8_t i = 0; i < len; i++)
        data[i] = SPI_Transfer(0xFF);
    CS_HIGH();
}

uint8_t BME280_init(void)
{
	// Шаг 1: Проверка Chip ID
    uint8_t chip_id = BME280_ReadByte(BME280_REG_ID);// должно вернуть 0x60

    if (chip_id != BME280_CHIP_ID)
		{
        return 1; // Датчик не найден
    }
		Delay_us(50000);
    // Шаг 2: Настройка режимов (нормальный режим, oversampling)
    BME280_WriteByteInReg(BME280_REG_CTRL_MEAS, 0x27); // temp: x2, press: x16, normal mode
		Delay_us(100000);
    // Шаг 3: Чтение калибровочных данных
    BME280_ReadCalibrationData();

    return 0; // Успех
}

void BME280_ReadCalibrationData(void)
{
    // Чтение коэффициентов температуры
    dig_T1 = (uint16_t)(BME280_ReadByte(0x88) | (BME280_ReadByte(0x89) << 8));
    dig_T2 = (int16_t)(BME280_ReadByte(0x8A) | (BME280_ReadByte(0x8B) << 8));
    dig_T3 = (int16_t)(BME280_ReadByte(0x8C) | (BME280_ReadByte(0x8D) << 8));

    // Чтение коэффициентов давления
    dig_P1 = (uint16_t)(BME280_ReadByte(0x8E) | (BME280_ReadByte(0x8F) << 8));
    dig_P2 = (int16_t)(BME280_ReadByte(0x90) | (BME280_ReadByte(0x91) << 8));
    dig_P3 = (int16_t)(BME280_ReadByte(0x92) | (BME280_ReadByte(0x93) << 8));
    dig_P4 = (int16_t)(BME280_ReadByte(0x94) | (BME280_ReadByte(0x95) << 8));
    dig_P5 = (int16_t)(BME280_ReadByte(0x96) | (BME280_ReadByte(0x97) << 8));
    dig_P6 = (int16_t)(BME280_ReadByte(0x98) | (BME280_ReadByte(0x99) << 8));
    dig_P7 = (int16_t)(BME280_ReadByte(0x9A) | (BME280_ReadByte(0x9B) << 8));
    dig_P8 = (int16_t)(BME280_ReadByte(0x9C) | (BME280_ReadByte(0x9D) << 8));
    dig_P9 = (int16_t)(BME280_ReadByte(0x9E) | (BME280_ReadByte(0x9F) << 8));
	
		// --- Влажность ---
    // Читаем первый коэффициент
    dig_H1 = BME280_ReadByte(0xA1);
    
    // Читаем блок коэффициентов во второй области памяти
    dig_H2 = (int16_t)(BME280_ReadByte(0xE1) | (BME280_ReadByte(0xE2) << 8)); // E1-E2
    dig_H3 = BME280_ReadByte(0xE3);

    // Сложная сборка для dig_H4 и dig_H5
    int16_t h4_lsb = BME280_ReadByte(0xE4);
    int16_t h5_msb = BME280_ReadByte(0xE5);
    uint8_t h4_h5_shared = BME280_ReadByte(0xE6); // Этот байт содержит старшие биты H4 и младшие биты H5

    dig_H4 = (h4_lsb << 4) | (h4_h5_shared & 0x0F); // Берем младшие 4 бита из регистра E6
    dig_H5 = (h5_msb << 4) | (h4_h5_shared >> 4);   // Берем старшие 4 бита из регистра E6

    dig_H6 = (int8_t)BME280_ReadByte(0xE7);
}

void BMP280_ReadBytes(uint8_t reg, uint8_t *buffer, uint8_t len)
{
		for (int i = 0; i < len; i++)
		{
				buffer[i] = BME280_ReadByte(reg++);
		}
		

}


// Запуск измерения и чтение сырых значений
void BME280_Measure(BME280_RawData_t *raw)
{
    // Запись CTRL_MEAS уже была сделана в init, но если ты хочешь менять режимы на лету,
    // здесь можно снова записать нужный режим.
    // Сейчас просто ждём завершения конверсии.
    Delay_us(200000); // >85 мс для oversampling x2/x16
		
		uint8_t buffer_len = 8;
    // Чтение 6 байт: T_MSB, T_LSB, T_XLSB, P_MSB, P_LSB, P_XLSB + 2 байта для влажности
    uint8_t buffer[buffer_len];
    // Адрес начала данных: BMP280_REG_PRESS_MSB 0xF7 
    BMP280_ReadBytes(BME280_REG_PRESS_MSB, buffer, buffer_len);

    // Сборка 20-битного значения температуры
    raw->raw_pressure = ((uint32_t)buffer[0] << 12) |
                           ((uint32_t)buffer[1] << 4)  |
                           ((buffer[2] >> 4) & 0x0F);

    // Сборка 20-битного значения давления
    raw->raw_temperature = ((uint32_t)buffer[3] << 12) |
                        ((uint32_t)buffer[4] << 4)  |
                        ((buffer[5] >> 4) & 0x0F);
	
		raw->raw_humidity = ((uint16_t)buffer[6] << 8) | buffer[7];
}

static uint32_t BMP280_compensate_P_int32(uint32_t adc_p, int32_t t_fine)
{
	int32_t var1, var2;
	uint32_t p;
	
	var1 = (((int32_t)t_fine >> 1) - ((int32_t)64000));
	
	
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)dig_P6);
var2 = var2 + ((var1 * (int32_t)dig_P5) << 1);
var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);

var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
var1 = ((((32768 + var1)) * ((int32_t)dig_P1)) >> 15);
	if (var1 == 0)
	{
		return 0;
	}
p = (((uint32_t)(((int32_t)1048576) - adc_p) - (var2 >> 12))) * 3125;
if (p < 0x80000000)
{
		p = (p << 1) / ((uint32_t)var1);
}
else
{
	p = (p / (uint32_t)var1) * 2;
}
var1 = (((int32_t)dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13)))>>12;
var2 = (((int32_t)(p >> 2)) * ((int32_t)dig_P8)) >> 13;
p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));

uint32_t pressure_rt_st = (uint32_t)round((p / 133.322f));//1 мм рт. ст.=133,322 Па
return pressure_rt_st;
}

static float BMP280_compensate_H_int32(int32_t adc_H, int32_t t_fine)
{
		float var_H;
    var_H = (float)t_fine - 76800.0f;
    var_H = (adc_H - (((float)dig_H4) * 64.0f + ((float)dig_H5) / 16384.0f *
			var_H)) * (((float)dig_H2) / 65536.0f * (1.0f + ((float)dig_H6) / 
			67108864.0f * var_H * 
			(1.0f + ((float)dig_H3) / 67108864.0f * var_H)));	
	
    var_H = var_H * (1.0f - ((float)dig_H1) * var_H / 524288.0f);


		if (var_H > 100.0f)
				return 100.0f;
    
		if (var_H < 0.0f)
        return 0.0f;
    
		return var_H;
}

void BME280_Compensate(BME280_RawData_t *raw, BME280_Result_t *result)
{
		int32_t var1, var2;
    int32_t adc_T = (int32_t)raw->raw_temperature;
    int32_t adc_P = (int32_t)raw->raw_pressure;
		int32_t adc_H = (int32_t)raw->raw_humidity;

    // ---------- Temperature ----------
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) *
              ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
             ((int32_t)dig_T3)) >> 14;
    int32_t t_fine = var1 + var2;
    result->temperature_c = (float)t_fine / 5120.0f;
		// ---------- Pressure ----------
		result->pressure_mm_rt_st = BMP280_compensate_P_int32(adc_P, t_fine);
		// ---------- Humidity ----------
		result->humidity = BMP280_compensate_H_int32(adc_H, t_fine);
}

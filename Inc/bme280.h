#ifndef __BME280_H
#define __BME280_H


#include "stm32f10x.h"

#define BME280_CHIP_ID   0x60   // для BME280



//#define I2C_TIMEOUT 20000



// Регистры BMP280
#define BME280_REG_ID    0xD0


#define BME280_REG_RESET 0xE0
#define BME280_REG_CTRL_MEAS 0xF4

#define BME280_REG_CONFIG 0xF5

#define BME280_REG_TEMP_MSB	 	0xFA
#define BME280_REG_TEMP_LSB  	0xFB
#define BME280_REG_TEMP_XLSB  0xFC

#define BME280_REG_PRESS_MSB 	0xF7
#define BME280_REG_PRESS_LSB 	0xF8
#define BME280_REG_PRESS_XLSB 0xF9


#define BME280_REG_HUM_MSB 		0xFD
#define BME280_REG_HUM_LSB		0xFE

// Структура для результатов
typedef struct {
    float temperature_c; // Температура в градусах Цельсия
    uint32_t pressure_mm_rt_st;   // Давление в мм рт ст
		float humidity;
} BME280_Result_t;
	
// Структура для хранения сырых данных
typedef struct {
    uint32_t raw_pressure;
    uint32_t raw_temperature;
		uint32_t raw_humidity;
} BME280_RawData_t;




void BME280_gpio_init(void);
uint8_t SPI_Transfer(uint8_t data);
uint8_t BME280_ReadByte(uint8_t reg);
void BME280_WriteByteInReg(uint8_t reg, uint8_t data);
void BME280_ReadBytes(uint8_t reg, uint8_t *data, uint8_t len);
uint8_t BME280_init(void);
void BME280_ReadCalibrationData(void);
uint8_t BME280_ReadData(float *temp, float *press);
void BME280_Measure(BME280_RawData_t *raw);
void BME280_Compensate(BME280_RawData_t *raw, BME280_Result_t *result);

#endif

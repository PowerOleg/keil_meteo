#ifndef __UART_H
#define __UART_H


#include "stm32f10x.h"


extern uint8_t txBuffer[];// = "Hello\r\n";//"Message sent from STM32 via DMA!\r\n";
extern char hello[];


extern uint8_t rxBuffer[];
extern volatile uint32_t last_received_time;

//extern volatile uint8_t receive_flag;
extern uint16_t buffer_index;// Текущая позиция в буфере
//extern volatile uint32_t receive_count;
extern volatile uint8_t transmission_in_progress;




void Uart2_init(void);
void Uart2_receive_string(void);
void Uart2_send_string_simple(char *str);
void Uart2_send_string(const char *str, uint16_t size);
//void Uart_send_string(const char *str);
void Init_dma_uart1(uint8_t *txBuffer, uint8_t size);
void Uart2_send_string_optimized(const char *str, const uint16_t size);
uint8_t Get_char_from_buffer(void);

#endif

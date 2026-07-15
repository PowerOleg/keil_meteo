#include <string.h>
#include "common.h"
#include "uart.h"
#include "misc.h"
#include "stm32f10x_usart.h"

volatile uint16_t tx_remaining_bytes = 0;
volatile uint8_t *tx_buffer_ptr = NULL;
volatile uint8_t transmission_in_progress = 0;

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_TXE) == SET)
		{
        if (tx_remaining_bytes > 0)
				{
            USART_SendData(USART2, *tx_buffer_ptr++);
						tx_remaining_bytes--;
        }
				else
				{
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
						transmission_in_progress = 0;
        }
    }
}

void Uart2_send_string(const char *str, const uint16_t size)
{
		tx_buffer_ptr = (uint8_t *)str;
    tx_remaining_bytes = size;

    // Начало передачи первого байта и разрешение прерывания
    USART_SendData(USART2, *tx_buffer_ptr++);
    tx_remaining_bytes--;
		transmission_in_progress = 1;
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

void Uart2_init(void)
{
		USART_DeInit(USART2);
		GPIO_InitTypeDef gpio_init_struct;
		gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
		gpio_init_struct.GPIO_Pin = GPIO_Pin_2;//к RX USB TTL
		gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio_init_struct);
		
		gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpio_init_struct.GPIO_Pin = GPIO_Pin_3;//к TX USB-TTL
		GPIO_Init(GPIOA, &gpio_init_struct);
		
		USART_InitTypeDef uart_init = {0};
		uart_init.USART_BaudRate = 9600;
		uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		uart_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
		uart_init.USART_Parity = USART_Parity_No;
		uart_init.USART_StopBits = USART_StopBits_1;
		uart_init.USART_WordLength = USART_WordLength_8b;
		USART_Init(USART2, &uart_init);
	//	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);//буфер пуст поэтому не включаем
		
		NVIC_InitTypeDef nvic_init = {0};
		nvic_init.NVIC_IRQChannel = USART2_IRQn;
		nvic_init.NVIC_IRQChannelPreemptionPriority = NVIC_UART_PRIORITY;
		nvic_init.NVIC_IRQChannelSubPriority = 0;
		nvic_init.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvic_init);

		USART_Cmd(USART2, ENABLE);
}

void Uart2_send_string_simple(char *str)
{
		while (*str)
		{
			while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}
			USART_SendData(USART2, *str++);
		}
}

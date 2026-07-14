#include <string.h>
#include "common.h"
#include "uart.h"
#include "misc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "timer.h"

#define UART_BUFFER_SIZE 64

//uint8_t uart2_tx_buffer[UART_BUFFER_SIZE];// = "Hello\r\n";//"Message sent from STM32 via DMA!\r\n";
uint8_t uart2_rx_buffer[UART_BUFFER_SIZE];
uint16_t buffer_index = 0;// Текущая позиция в буфере uart2_rx_buffer

volatile uint32_t last_received_time = 0;
//volatile uint8_t receive_flag = 0;
//volatile uint32_t receive_count = 0;

volatile uint16_t tx_remaining_bytes = 0;

/*volatile uint8_t *tx_buffer_ptr = NULL;
volatile uint16_t tx_index = 0;
*/
volatile uint8_t transmission_in_progress = 0;

#define TX_BUFFER_SIZE 1024
volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
volatile uint16_t head = 0;
volatile uint16_t tail = 0;


void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_TXE) == SET) {
        if (tx_remaining_bytes > 0) {
            USART_SendData(USART2, Get_char_from_buffer());
						tx_remaining_bytes--;
        } else {
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
						transmission_in_progress = 0;
        }
    }
}

uint8_t Get_char_from_buffer(void)
{
    uint8_t c = tx_buffer[tail];
    tail = (tail + 1) % TX_BUFFER_SIZE;
    return c;
}



void Put_char_into_buffer(uint8_t c)
{
    tx_buffer[head] = c;
    head = (head + 1) % TX_BUFFER_SIZE;
}

void Uart2_send_string(const char *str, uint16_t size)
{
	
    if (size > TX_BUFFER_SIZE) {
        // Опционально: отправить только первые BUFFER_SIZE байт
        size = TX_BUFFER_SIZE;
        // Или просто return с сообщением об ошибке
    }

    tx_remaining_bytes = size;
    for (uint16_t i = 0; i < size; i++) {
        Put_char_into_buffer(str[i]);
    }
    transmission_in_progress = 1;
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}









/*
static bool Is_buffer_empty(void)
{
    return head == tail;
}
void Uart2_send_string(const char *str, const uint16_t size)
{
		tx_remaining_bytes = size;
    while (tx_remaining_bytes > 0) 
		{
        Put_char_into_buffer(*str++);
				tx_remaining_bytes--;
    }
		tx_remaining_bytes = size;
		transmission_in_progress = 1;
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
}

*/




/**
 * @brief Улучшенная функция отправки строки через UART
 *
 * @param str Указатель на строку
 */
/*void Uart2_send_string_optimized(char *str)
{
    static const uint16_t BUFFER_SIZE = 64;
    char send_buffer[BUFFER_SIZE];
    uint16_t remaining = strlen(str);
    const char* ptr = str;

    while (remaining > 0) {
        uint16_t chunk_size = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : remaining;
        memcpy(send_buffer, ptr, chunk_size);
        send_buffer[chunk_size] = '\0';

        while (*send_buffer) {
            while (!(USART2->SR & USART_SR_TXE));
            USART2->DR = (*send_buffer)++;
        }

        ptr += chunk_size;
        remaining -= chunk_size;
    }
}*/


/*
void Uart2_send_string_optimized(const char *str, const uint16_t size)
{
    __disable_irq(); // Блокировка прерываний для предотвращения конфликтов

    if (!transmission_in_progress) {
        transmission_in_progress = true;

        tx_buffer_ptr = (uint8_t *)str;
        tx_remaining_bytes = size;
        tx_index = 0;

        // Начало передачи первого байта и разрешение прерывания
        USART_SendData(USART2, tx_buffer_ptr[tx_index++]);
        tx_remaining_bytes--;
        USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
    }

    __enable_irq(); // Возобновление прерываний
}
*/









//для получения
/*void USART2_IRQHandler(void)
{
    // Проверяем, вызвано ли прерывание (RXNE)
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        // Это действие автоматически сбрасывает флаг прерывания.
        uint8_t receivedChar = USART_ReceiveData(USART2);
				if (buffer_index < UART_BUFFER_SIZE - 1) 
				{
						uart2_rx_buffer[buffer_index++] = receivedChar;
				}
			
				// Фиксируем время прихода последнего байта
        last_received_time = timer2_cur_time_ms;
			
				// Сбрасываем счетчик буфера, если он переполнен (опционально)
        if (buffer_index >= UART_BUFFER_SIZE - 1)
            buffer_index = UART_BUFFER_SIZE - 1;
    }
}*/




/*//task3_SPL
void USART1_IRQHandler(void)
{
    // 1. Проверяем, произошло ли IDLE-прерывание
    if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET)
    {
        // 2. Очищаем флаг IDLE-прерывания.
        // Важно: сначала читаем SR, затем DR, чтобы сбросить флаг IDLE.
        
				volatile uint32_t tmp = USART1->SR;
        tmp = USART1->DR;

        // 3. Вычисляем длину принятых данных.
        // CNDTR показывает, сколько элементов осталось принять DMA.
        data_length = RX_DMA_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);

        // 4. Устанавливаем флаг для обработки в main
        data_received_flag = 1;
    }

    // if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) { ... }
}
*/


void Uart2_receive_string(void)
{
	 // Проверяем: если буфер не пуст И прошло больше времени, чем наш таймаут
	//если байты приходят подряд то мы вписываемся в диапазон TIMEOUT_END_200_MS и тогда не входим в блок
        if (buffer_index > 0 && ((timer2_cur_time_ms - last_received_time) >= TIMEOUT_END_200_MS))
        {
									
            uint16_t messageLength = buffer_index;
            buffer_index = 0; // Очищаем индекс для нового приема

            uart2_rx_buffer[messageLength] = '\0'; // Завершаем строку
            // Отправка данных
            for(int i = 0; i < strlen((char*)uart2_rx_buffer); i++)
            {
                while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
                USART_SendData(USART2, uart2_rx_buffer[i]);
            }
            while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        }						
}

//Для отправки
void Uart2_send_string_simple(char *str)
{
//	char ch = *str;
	while (*str)
  {
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}
    USART_SendData(USART2, *str++);
  }
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
	
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);//
	
	NVIC_InitTypeDef nvic_init = {0};
	nvic_init.NVIC_IRQChannel = USART2_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = NVIC_UART_PRIORITY;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);

	USART_Cmd(USART2, ENABLE);
}




void Init_dma_uart1(uint8_t *uart2_tx_buffer, uint8_t size)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	//PA9 - TX
	GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.GPIO_Pin = GPIO_Pin_9;//к RX USB TTL
    gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_init_struct);
	//PA10 - RX
		gpio_init_struct.GPIO_Pin = GPIO_Pin_10;//к TX USB TTL
		gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio_init_struct);
	
	USART_DeInit(USART1);
	USART_InitTypeDef uart_init = {0};
	uart_init.USART_BaudRate = 9600;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	uart_init.USART_Parity = USART_Parity_No;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &uart_init);
	
	DMA_DeInit(DMA1_Channel4);
	DMA_InitTypeDef dma1_usart1_init = {0};
	dma1_usart1_init.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	dma1_usart1_init.DMA_MemoryBaseAddr = (uint32_t)uart2_tx_buffer;
	dma1_usart1_init.DMA_DIR = DMA_DIR_PeripheralDST;
	dma1_usart1_init.DMA_BufferSize = size;
	dma1_usart1_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma1_usart1_init.DMA_MemoryInc = DMA_PeripheralInc_Enable;
	dma1_usart1_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dma1_usart1_init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	
	dma1_usart1_init.DMA_Mode = DMA_Mode_Normal;     // Режим: однократная передача (не циклический)
  dma1_usart1_init.DMA_Priority = DMA_Priority_High; // Приоритет высокий
  dma1_usart1_init.DMA_M2M = DMA_M2M_Disable;       // Режим память-память выключен
  DMA_Init(DMA1_Channel4, &dma1_usart1_init);
	
	
	
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	USART_Cmd(USART1, ENABLE);
//	DMA_Cmd(DMA1_Channel4, ENABLE);
	

    NVIC_InitTypeDef nvic_init;
    nvic_init.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = NVIC_UART_DMA_PRIORITY;
    nvic_init.NVIC_IRQChannelSubPriority = 0;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);


/*
DMA_DeInit(DMA1_Channel5);
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR; // Тот же адрес регистра данных UART
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)uart2_rx_buffer;       // Адрес буфера приема в RAM
    
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;              // Направление: Периферия -> Память
    
    DMA_InitStructure.DMA_BufferSize = RX_BUFFER_SIZE;               // Размер кольцевого буфера
    
    //Для приема важно включить инкремент адреса памяти и циклический режим
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                 // Циклический режим!
    
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
		
		//Включаем прерывания от канала DMA1 Channel5 (RX Complete - если нужно)
     nvic_init.NVIC_IRQChannel = DMA1_Channel5_IRQn;
     NVIC_Init(&nvic_init);
		*/
}


/*
void DMA1_Channel4_IRQHandler(void)
{
  // Проверяем, действительно ли прерывание вызвано завершением передачи 
  if (DMA_GetITStatus(DMA1_IT_TC4))
  {
      // Сбрасываем флаг прерывания. Это ОБЯЗАТЕЛЬНО нужно сделать! 
      DMA_ClearITPendingBit(DMA1_IT_TC4);
      
      // Здесь можно добавить код, который выполнится после отправки данных 
      //Например, выключить DMA или зажечь светодиод 
      // GPIO_SetBits(GPIOC, GPIO_Pin_8); 
      
      // Отключаем канал DMA, если он больше не нужен 
      DMA_Cmd(DMA1_Channel4, DISABLE); 
  }
}
*/


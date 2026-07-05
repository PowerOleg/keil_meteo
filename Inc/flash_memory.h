#ifndef __FLASH_MEMORY_H
#define __FLASH_MEMORY_H

#include "stm32f10x.h"
#include "stm32f10x_flash.h"

void Flash_write_string(const char* str);
void Flash_read_string(char* buffer, uint16_t maxLen);


#endif

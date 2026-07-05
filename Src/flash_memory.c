#include "flash_memory.h"
#include <string.h>


#define FLASH_USER_START_ADDR   0x0800FC00  // последн€€ страница 1K
#define FLASH_PAGE_SIZE         1024

// ‘ункци€ записи строки во Flash
void Flash_write_string(const char* str)
{
    uint32_t addr = FLASH_USER_START_ADDR;
    uint16_t data;
    int len = strlen(str) + 1; // с нуль-терминатором
    int i;

    FLASH_Unlock();
    FLASH_ErasePage(FLASH_USER_START_ADDR);

    // ѕишем полусловами (16 бит), собира€ байты попарно
    for (i = 0; i < len; i += 2) {
        data = (uint16_t)str[i];
        if (i + 1 < len)
            data |= (uint16_t)str[i + 1] << 8;
        FLASH_ProgramHalfWord(addr, data);
        addr += 2;
    }

    FLASH_Lock();
}

// ‘ункци€ чтени€ строки из Flash
void Flash_read_string(char* buffer, uint16_t maxLen)
{
    uint32_t addr = FLASH_USER_START_ADDR;
    uint16_t data;
    int i = 0;

    while (i < maxLen - 1) {
        data = *(__IO uint16_t*)addr;
        buffer[i] = (char)(data & 0xFF);
        if (buffer[i] == '\0') break;
        i++;
        if (i >= maxLen - 1) break;
        buffer[i] = (char)(data >> 8);
        if (buffer[i] == '\0') break;
        i++;
        addr += 2;
    }
    buffer[i] = '\0';
}


#include "flash_control.h"
#include "stdint.h"
#include "stm32u5xx_it.h"
#include <string.h>
#include "stm32u5xx_hal.h"
#include "stdio.h"


void erase_bank(uint32_t memory_address, uint16_t num_pages)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase = {0};
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    // Page number = offset from FLASH_BASE divided by page size
    erase.Page      = (memory_address - FLASH_BASE) / FLASH_PAGE_SIZE;
    erase.NbPages   = num_pages;
    erase.Banks     = FLASH_BANK_OF(memory_address);

    uint32_t page_error = 0;
    if (HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK)
    {
        // handle error: page_error holds the failing page index
    }

    HAL_FLASH_Lock();
}


void store_flash_memory(uint32_t memory_address, uint8_t *data, uint16_t length)
{
    HAL_FLASH_Unlock();
    uint32_t offset = 0;
    uint8_t  tmp[16];

    // 1) full 16-byte chunks
    while (length - offset >= 16)
    {
        memcpy(tmp, data + offset, 16);
        // pass the address of tmp (16 bytes) to the quad-word API
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD,
        		memory_address + offset,
                          (uint64_t)(uintptr_t)tmp);
        offset += 16;
    }

    if (length > offset)
    {
        uint32_t rem = length - offset;
        memset(tmp, 0xFF, sizeof(tmp));
        memcpy(tmp, data + offset, rem);

        HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD,
        		memory_address + offset,
                          (uint64_t)(uintptr_t)tmp);
    }

    HAL_FLASH_Lock();
}


void toggle_bank(void)
{
    FLASH_OBProgramInitTypeDef ob = {0};
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPTWERR);

    // 1) Unlock flash and option bytes
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP   |
                           FLASH_FLAG_OPTWERR);
    HAL_FLASH_OB_Unlock();

    // 2) Read current OB config
    HAL_FLASHEx_OBGetConfig(&ob);

    ob.OptionType  = OPTIONBYTE_USER;
    ob.USERType    = OB_USER_SWAP_BANK;
    ob.USERConfig  = (ob.USERConfig & OB_SWAP_BANK_ENABLE)
                       ? OB_SWAP_BANK_DISABLE
                       : OB_SWAP_BANK_ENABLE;
    HAL_FLASHEx_OBProgram(&ob);

    HAL_FLASH_OB_Launch();
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
}



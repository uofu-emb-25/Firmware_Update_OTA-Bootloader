#ifndef INC_FLASH_CONTROL_U5_H_
#define INC_FLASH_CONTROL_U5_H_

#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_flash_ex.h"
#include "stm32u5xx_hal_def.h"

// U5 page size is fixed at 8 KB
#define FLASH_PAGE_SIZE       0x2000U

// On a 2 MB-device each bank is 1 MB
#define FLASH_BANK2_BASE        (FLASH_BASE + FLASH_BANK_SIZE)

// Determine bank by address
#define FLASH_BANK_OF(addr)     (((addr) < FLASH_BANK2_BASE) ? FLASH_BANK_1 : FLASH_BANK_2)


void erase_bank(uint32_t memory_address, uint16_t num_pages);
void store_flash_memory(uint32_t memory_address, uint8_t *data, uint16_t length);
void toggle_bank();
void Bootloader_CheckAndJump(void);

#endif /* INC_FLASH_CONTROL_U5_H_ */

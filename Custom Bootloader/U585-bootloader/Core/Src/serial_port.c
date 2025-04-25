#include "serial_port.h"
#include "stdio.h"
#include "flash_control.h"

#define BANK_PAGES  (FLASH_BANK_SIZE / FLASH_PAGE_SIZE)

extern CRC_HandleTypeDef hcrc;
uint8_t count = 0;
static uint32_t  fw_base_addr;
static uint32_t  fw_write_offset;

uint8_t boot_verify_crc(uint8_t *data, uint8_t len, uint32_t crc_host)
{
	uint32_t crc_value = 0xFFFFFFFF;
	for(uint32_t i = 0; i < len; i++)
	{
		uint32_t i_data = data[i];
		crc_value = HAL_CRC_Accumulate(&hcrc, &i_data, 1);
	}
	 __HAL_CRC_DR_RESET(&hcrc);
	if (crc_value == crc_host)
	{
		printf("CRCs match each other \n");
		return 0;
	}
	printf("CRC doesn't match : : Firmware Update Failed! \n");
	return 1;

}


static uint32_t get_inactive_bank_base(void)
{
    FLASH_OBProgramInitTypeDef obcfg = {0};
    HAL_FLASHEx_OBGetConfig(&obcfg);

    if (obcfg.USERConfig & OB_SWAP_BANK_ENABLE) {
        // bank2 is at 0x0800_0000, so bank1 is inactive
        return FLASH_BASE;
    } else {
        // bank1 is at 0x0800_0000, so bank2 is inactive
        return FLASH_BANK2_BASE;
    }
}


uint8_t update_firmware(uint8_t *data, uint8_t size){
    // Extract host CRC (last 4 bytes, little-endian)
    uint16_t payload_len = size - 4;
    uint32_t crc_host =  (uint32_t)data[payload_len]
                       | ((uint32_t)data[payload_len + 1] << 8)
                       | ((uint32_t)data[payload_len + 2] << 16)
                       | ((uint32_t)data[payload_len + 3] << 24);

    // Verify CRC
    if (boot_verify_crc(data, payload_len, crc_host)) {
        return 1;  // CRC mismatch
    }

    // On first chunk, erase the inactive bank
    count++;
    if (count == 1) {
        fw_base_addr    = get_inactive_bank_base();
        fw_write_offset = 0;
        erase_bank(fw_base_addr, BANK_PAGES);
    }

    // If this is the final marker (e.g., size == 5), print VT and swap banks
    if (size == 5) {
        uint32_t app_base = get_inactive_bank_base();
        uint32_t *vt = (uint32_t *)app_base;
        printf("SP: 0x%08lX, Reset Handler: 0x%08lX\n", vt[0], vt[1]);

        toggle_bank();  // Swaps banks and resets
    } else {
        // Write payload to flash
        uint32_t addr = fw_base_addr + fw_write_offset;
        store_flash_memory(addr, data, payload_len);

        // Pad to 16-byte boundary if needed
        uint32_t padded = ((payload_len + 15) / 16) * 16;
        fw_write_offset += padded;
    }

    return 0;
//	// Check the CRC first before updating
//	count += 1;
//	if(boot_verify_crc(data, size - 4 , *((uint32_t *)&data[size - 4]))){
//		return 1;
//	}
//
//	//Erase Bank
//	if (count == 1){
//		fw_base_addr = get_inactive_bank_base();
//		fw_write_offset = 0;
////		uint32_t bank_bytes = FLASH_BANK_SIZE; //* 1024U;
////		uint16_t pages = (bank_bytes + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
////		erase_bank(fw_base_addr, pages);
//		erase_bank(fw_base_addr, BANK_PAGES);
//
//
//	}
//
//	//Store
//
//	if (size == 5){
//		uint32_t app_base = get_inactive_bank_base();  // e.g., 0x08100000
//		uint32_t *vt = (uint32_t *)app_base;
//
//		printf("SP: 0x%08lX, Reset Handler: 0x%08lX\n", vt[0], vt[1]);
//
//		toggle_bank();
//	}
//	else
//	{
//	uint32_t addr = fw_base_addr + fw_write_offset;
//	store_flash_memory(addr, data, (size - 4));
//	fw_write_offset += (size - 4);
//	}
//
//
//	return 0;
}


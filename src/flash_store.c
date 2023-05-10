#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

#include "flash_store.h"
 
int wifi_details_clear() {
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    return WIFI_DETAILS_UNSET;
}

int wifi_details_save(char *ssid, char *pwd) {
    uint8_t buf[FLASH_SECTOR_SIZE];

    uint32_t pos = 0, len;

    // GLOW Magic Number
    buf[pos++] = 0x47;
    buf[pos++] = 0x4C;
    buf[pos++] = 0x4F;
    buf[pos++] = 0x57;

    // Write 1 byte length then string
    len = buf[pos++] = strlen(ssid);
    strcpy(buf+pos, ssid);
    pos += len;

    // Write 1 byte length then string
    len = buf[pos++] = strlen(pwd);
    strcpy(buf+pos, pwd);
    pos += len;

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
    flash_range_program(PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE, (uint8_t *)buf, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
    
    return WIFI_DETAILS_OK;
}

int wifi_details_available() {
    uint8_t *buf = (uint8_t *)(XIP_BASE + PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE);

    if((buf[0]!=0x47) || (buf[1]!=0x4C) || (buf[2]!=0x4F) || (buf[3]!=0x57)) return WIFI_DETAILS_UNSET;

    return WIFI_DETAILS_OK;
}

int wifi_details_load(char *ssid, char *pwd) {
    uint8_t *buf = (uint8_t *)(XIP_BASE + PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE);

    if((buf[0]!=0x47) || (buf[1]!=0x4C) || (buf[2]!=0x4F) || (buf[3]!=0x57)) return WIFI_DETAILS_UNSET;

    uint32_t pos = 4, len;
    len = buf[pos++];
    for(uint8_t i = 0; i<len; i++) {
        *(ssid++) = buf[pos++];
    }
    *(ssid++) = 0;

    len = buf[pos++];
    for(uint8_t i = 0; i<len; i++) {
        *(pwd++) = buf[pos++];
    }
    *(pwd++) = 0;

    return WIFI_DETAILS_OK;
}
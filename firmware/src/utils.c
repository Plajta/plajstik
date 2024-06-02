#include <string.h>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "tiny-json.h"
#include "utils.h"
#include "maps.h"

size_t strlen_s(const char *str, size_t max_len) { // Replacement for the somehow missing strlen_s function
    const char * end = (const char *)memchr(str, '\0', max_len);
    if (end == NULL)
        return max_len;
    else
        return end - str;
}


int save_string(uint32_t target_addr, char* buf, int max_len){
    if (target_addr + max_len > 0x200000)
        return 1; // Out of bounds
    if ((target_addr % FLASH_SECTOR_SIZE != 0) || (max_len % FLASH_PAGE_SIZE != 0))
        return 2; // Wrongly aligned address or max_len

    unsigned int sectors = (max_len + (FLASH_SECTOR_SIZE-1)) / FLASH_SECTOR_SIZE;

    char pversion_aligned[max_len];

    memset(pversion_aligned, '\0', max_len);
    strcpy(pversion_aligned, buf);

    pversion_aligned[max_len-1] = '\0'; // Just to be sure

    uint32_t ints = save_and_disable_interrupts(); // Save interrupts so they can be restored after altering flash, this is needed due to the usversion of USB
    flash_range_erase(target_addr, FLASH_SECTOR_SIZE*sectors);
    flash_range_program(target_addr, pversion_aligned, max_len);
    restore_interrupts(ints);

    return 0;
}

// Parse JSON from string and test if it is valid
int json_validity(char* buf){
    json_t mem[32]; // This buffer size should be enough because the RP2040 only has 30 IO pins
    json_t const* json = json_create(buf, mem, sizeof mem / sizeof *mem);
    if ( !json ) {
        return 1;
    }
    else{
        json_t const* version = json_getProperty( json, "version" );
        if (!version || json_getType( version ) != JSON_INTEGER) {
            return 1;
        }
        int const versionVal = (int)json_getInteger( version );
        if (versionVal != JSON_VERSION){
            return 2;
        }
    }
    return 0;
}

void json_setup(char* buf, uint8_t* keymap, uint8_t* keymap_dpad){
    json_t mem[32]; // This buffer size should be enough because the RP2040 only has 30 IO pins
    json_t const* json = json_create(buf, mem, sizeof mem / sizeof *mem);
    json_t const* child;
    json_t const* parent = json_getProperty(json, "buttons");
    for(child = json_getChild(parent); child != 0; child = json_getSibling(child)) {
        int8_t pin = (int)json_getInteger(child);
        int8_t key = find_mapping(json_getName(child));
        if (key > -1) keymap[key] = pin;
        else {
            key = find_dpad(json_getName(child));
            if (key > -1) keymap_dpad[key] = pin;
        }
    }
}

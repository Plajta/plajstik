#include <stdint.h>
#include <string.h>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "tiny-json.h"
#include "utils.h"
#include "maps.h"
#include "tusb.h"

size_t strlen_s(const char *str, size_t max_len) { // Replacement for the somehow missing strlen_s function
    const char * end = (const char *)memchr(str, '\0', max_len);
    if (end == NULL)
        return max_len;
    else
        return end - str;
}

int16_t clamp(int16_t d, int16_t min, int16_t max) {
  const int16_t t = d < min ? min : d;
  return t > max ? max : t;
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

void json_setup(char* buf, int8_t* keymap, int8_t* keymap_dpad, double* deadzone, int8_t* x_adc, int8_t* y_adc, float* x_multiplier, float* y_multiplier){
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
    json_t const* deadzone_json = json_getProperty( json, "deadzone" );
    if (deadzone_json && json_getType( deadzone_json ) == JSON_REAL) {
        *deadzone = (double)json_getReal(deadzone_json);
    }

    parent = json_getProperty(json, "axes");

    if (parent) {
        json_t const* x_json = json_getProperty( parent, "x" );
        if (x_json && json_getType( x_json ) == JSON_INTEGER) {
            *x_adc = (int)json_getInteger( x_json );
        }

        json_t const* y_json = json_getProperty( parent, "y" );
        if (y_json && json_getType( y_json ) == JSON_INTEGER) {
            *y_adc = (int)json_getInteger( y_json );
        }
    }

    parent = json_getProperty(json, "multiplier");

    *x_multiplier = *y_multiplier = 1.0;

    if (parent) {
        json_t const* x_json = json_getProperty( parent, "x" );
        if (x_json && json_getType( x_json ) == JSON_REAL) {
            *x_multiplier = (float)json_getReal( x_json );
        }

        json_t const* y_json = json_getProperty( parent, "y" );
        if (y_json && json_getType( y_json ) == JSON_REAL) {
            *y_multiplier = (float)json_getReal( y_json );
        }
    }

}

void utils_tud_cdc_write_s(char character){
    if (character == '\n' || character == '\r'){
        tud_cdc_write_str("\n\r");
    }
    else{
        tud_cdc_write(&character, 1);
    }
}

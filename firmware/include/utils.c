#include <string.h>
#include "hardware/flash.h"
#include "hardware/sync.h"

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

    char page_aligned[max_len];

    memset(page_aligned, '\0', max_len);
    strcpy(page_aligned, buf);

    page_aligned[max_len-1] = '\0'; // Just to be sure

    uint32_t ints = save_and_disable_interrupts(); // Save interrupts so they can be restored after altering flash, this is needed due to the usage of USB
    flash_range_erase(target_addr, FLASH_SECTOR_SIZE*sectors);
    flash_range_program(target_addr, page_aligned, max_len);
    restore_interrupts(ints);

    return 0;
}

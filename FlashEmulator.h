#ifndef _FLASH_EMULATOR_H
#define _FLASH_EMULATOR_H

void flash_init(void);

int flash_format(void);
int flash_erase_sector(unsigned int addr);

int flash_read(void* dest, unsigned int size, unsigned int addr);
int flash_write(void* src, unsigned int size, unsigned int addr);

void pretty(void);

#endif

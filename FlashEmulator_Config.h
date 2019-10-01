#ifndef _FLASH_EMULATOR_CONFIG_H
#define _FLASH_EMULATOR_CONFIG_H

#define EMPTY_BYTE      ((unsigned char)0xFF)

#define KIB             (1024)
#define MIB             (KIB*KIB)

#define SECTOR_SIZE     (4*KIB)
#define FLASH_SIZE      (8*MIB)

#define SECTOR_COUNT    (FLASH_SIZE/SECTOR_SIZE)

#if (FLASH_SIZE % SECTOR_SIZE) != 0
#error "Flash size must be an integer multiple of sector size"
#endif

#define FLASH_FILENAME  "memory.bin"
#define PRETTY_FILENAME	"pretty.txt"

#define BYTES_IN_A_ROW  8

#endif

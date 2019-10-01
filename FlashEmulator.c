#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "FlashEmulator.h"
#include "FlashEmulator_Config.h"

#if PRINT_ERRORS
#define ERR(...) printf(__VA_ARGS__)
#else
#define ERR(...)
#endif

int flash_erase_sector(unsigned int addr)
{
	FILE* memory;
	uint8_t empty_sector[SECTOR_SIZE];

	if ( addr >= FLASH_SIZE )
	{
		ERR("%s invalid address %u\r\n",__func__,addr);
		return -2;
	}

	memory = fopen(FLASH_FILENAME,"rb+");

	if ( NULL == memory )
	{
		ERR("%s cannot open %s\r\n",__func__, FLASH_FILENAME);
		return -1;
	}

	printf("%s %08X\r\n", __func__, addr);

	memset(empty_sector, EMPTY_BYTE, SECTOR_SIZE);

	addr -= addr % SECTOR_SIZE;

	fseek(memory, addr, SEEK_SET);

	fwrite(empty_sector, 1, SECTOR_SIZE, memory);

	fclose(memory);
	
	return 0;
}

int flash_format(void)
{
	int i;
	uint8_t empty_sector[SECTOR_SIZE];
	FILE* memory;
	/*	Empty sector is full of 0xFF	*/
	memset(empty_sector, 0xFF, SECTOR_SIZE);
	/* 	Erase if exists, open for binary write */
	memory = fopen(FLASH_FILENAME, "wb");

	if ( NULL == memory )
	{
		ERR("Cannot create %s\r\n",FLASH_FILENAME);
		return -1;
	}

	fclose(memory);

	for (i=0; i<SECTOR_COUNT; ++i)
	{
		if (0 != flash_erase_sector(i*SECTOR_SIZE))
        {
            return -2;
        }
	}
    
    return 0;
}

void flash_init(void)
{
	FILE* memory;

	memory = fopen(FLASH_FILENAME, "rb");

	if ( NULL == memory )
	{
		flash_format();
	}
	else
	{
		fclose(memory);
	}
}

int flash_read(void* dest, unsigned int size, unsigned int addr)
{
	int i;
	unsigned int read;
	FILE* memory;

	if (addr >= FLASH_SIZE)
	{
		ERR("%s invalid address %u\r\n",__func__,addr);
		return -1;
	}
	if ( (addr+size) >= FLASH_SIZE )
	{
		ERR("%s invalid size %u\r\n",__func__,size);
		return -2;
	}

	memory = fopen(FLASH_FILENAME,"rb");

	if ( NULL == memory )
	{
		flash_format();
		memory = fopen(FLASH_FILENAME,"rb");
		if ( NULL == memory )
		{
			ERR("%s cannot open %s\r\n",__func__,FLASH_FILENAME);
			exit(-1);
		}
	}

	fseek(memory, addr, SEEK_SET);

	read = fread(dest, 1, size, memory);

	if ( read != size )
	{
		ERR("%s read error (%d/%d) %08X\r\n",__func__,read,size,addr);
		fclose(memory);
		return -3;
	}

#if PRINT_READ_DATA
	printf("%s(%p, %u, %08X):",__func__,dest,size,addr);
	for (i=0; i<size; ++i)
	{
		if ( 0 == (i%16) )
		{
			printf("\r\n\t");
		}
		printf("%02X ", dest[i]);
	}
	printf("\r\n");
#endif

	fclose(memory);

	return 0;
}

int flash_write(void* src, unsigned int size, unsigned int addr)
{
	uint8_t* buffer = NULL;
	unsigned int read, written;
	int i;
	FILE* memory;

	if (addr >= FLASH_SIZE)
	{
		ERR("%s invalid address %08X\r\n",__func__,addr);
		return -1;
	}
	if ( (addr+size) >= FLASH_SIZE )
	{
		ERR("%s invalid size %u\r\n",__func__,size);
		return -2;
	}

	memory = fopen(FLASH_FILENAME, "rb+");

	if ( NULL == memory )
	{
		ERR("%s cannot open %s\r\n",__func__,FLASH_FILENAME);
		exit(-1);
	}

	buffer = malloc(size);

	if ( NULL == buffer )
	{
		ERR("%s malloc failed\r\n",__func__);
		fclose(memory);
		exit(-1);
	}

	fseek(memory, addr, SEEK_SET);

	read = fread(buffer, 1, size, memory);
	if ( read != size )
	{
		ERR("%s read error (%u/%u)\r\n",__func__,read,size);
		fclose(memory);
		return -3;
	}

	for (i=0; i<size; ++i)
	{
		if ( EMPTY_BYTE != buffer[i] )
		{
			printf("Warning: %s() writing over unformated flash(%02X)\r\n", __func__, buffer[i]);
		}
		buffer[i] &= src[i];

		if ( buffer[i] != src[i] )
		{
			printf("Warning: %s() wrong data written (%02X, %02X)\r\n", __func__, buffer[i], src[i]);
		}
	}

	fseek(memory, addr, SEEK_SET);

	written = fwrite(buffer, 1, size, memory);
	if ( written != size )
	{
		ERR("%s write error (%u/%u) %08X\r\n",__func__, written, size, addr);
		fclose(memory);
		return -4;
	}
#if PRINT_DATA_TO_WRITE
	printf("%s data to write:",__func__);
	for (i=0; i<size; ++i)
	{
		if ( 0 == (i%16) )
		{
			printf("\r\n\t");
		}
		printf("%02X ",src[i]);
	}
	printf("\r\n");
#endif

#if PRINT_WRITTEN_DATA
	fflush(memory);

	fseek(memory, addr, SEEK_SET);

	fread(buffer, 1, size, memory);

	printf("%s(%p, %u, %08X):",__func__,src,size,addr);
	for (i=0; i<size; ++i)
	{
		if ( 0 == (i%16) )
		{
			printf("\r\n\t");
		}
		printf("%02X ", buffer[i]);
	}
	printf("\r\n");
#endif

	fclose(memory);

	return 0;
}

int pretty(void)
{
	int i,j,k;
	FILE* memory;
	FILE* pretty_memory;
	uint8_t buffer[BYTES_IN_A_ROW];

	memory = fopen(FLASH_FILENAME, "rb");
	if ( NULL == memory )
	{
		ERR("%s cannot open %s\r\n", __func__, FLASH_FILENAME);
		return -1;
	}

	pretty_memory = fopen(PRETTY_FILENAME, "w");
	if ( NULL == pretty_memory )
	{
		ERR("%s cannot create %s\r\n", __func__, PRETTY_FILENAME);
		fclose(memory);
		return -1;
	}
	fprintf(pretty_memory, "         ");
	for (i=0; i<(SECTOR_COUNT); ++i)
	{
		/*
			TODO
			Make this fmt string depend on BYTES_IN_A_ROW
		*/
		fprintf(pretty_memory, "%-017X", i*SECTOR_SIZE);
	}
	fprintf(pretty_memory, "\n");
	for (i=0; i<(SECTOR_SIZE/BYTES_IN_A_ROW); ++i)
	{
		fprintf(pretty_memory, "%08X ", i*BYTES_IN_A_ROW);
		for (j=0; j<SECTOR_COUNT; ++j)
		{
			fseek(memory, SECTOR_SIZE*j + i*BYTES_IN_A_ROW, SEEK_SET);
			fread(buffer, 1, BYTES_IN_A_ROW, memory);
			for (k=0; k<BYTES_IN_A_ROW; ++k)
			{
				fprintf(pretty_memory, "%02X", buffer[k]);
			}
			fprintf(pretty_memory, " ");
		}
		fprintf(pretty_memory, "\n");
	}
	fclose(memory);
	fclose(pretty_memory);
    
    return 0;
}

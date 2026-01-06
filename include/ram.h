#ifndef RAM_H
#define RAM_H

#include <stddef.h>
#include <stdint.h>

#define RAM_SIZE (128 * 1024 * 1024)

void ram_init();

void    ram_write8(uint32_t addr, uint8_t* buf, size_t size);
uint8_t ram_read8(uint32_t addr);

void*   get_ram_ptr(uint32_t addr);

#endif
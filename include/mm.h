#ifndef MM_H
#define MM_H

#include <stdint.h>
#include <stddef.h>

void mm_init();
uint32_t mm_malloc(uint32_t size);
void mm_free(uint32_t addr);
uint32_t fmalloc(uint32_t size);
void ffree(uint32_t addr);

#endif
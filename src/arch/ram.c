#include "ram.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

static uint8_t* pram = NULL;

void ram_init(){
    pram = mmap(NULL, RAM_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if(pram == MAP_FAILED){
        perror("[FATAL] Ram init failure\n");
        exit(1);
    }

    printf("[SUCCESS] Ram initialized: %d MB at %p\n", RAM_SIZE / (1024 * 1024), (void*)pram);
}

void* get_ram_ptr(uint32_t addr) {
    if(addr >= RAM_SIZE) {
        fprintf(stderr, "Segmentation fault: Out of bounds memory access at 0x%08x\n\r", addr);
        return NULL;
    }

    return (void*)(pram + addr);
}

void ram_write8(uint32_t addr ,uint8_t* buf, size_t size) {
    for(size_t i = 0; i < size; i++){
        uint8_t* ptr = (uint8_t*)get_ram_ptr(addr + i);
        if(ptr) *ptr = buf[i];
    }
}

uint8_t ram_read8(uint32_t addr) {
    uint8_t* ptr = (uint8_t*)get_ram_ptr(addr);
    return ptr ? *ptr : 0;
}
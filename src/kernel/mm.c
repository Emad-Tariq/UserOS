#include "ram.h"
#include "process.h"
#include "cpu.h"
#include "mm.h"
#include <stdio.h>

typedef struct block {
    uint32_t size;
    int free;
    struct block* next;
    struct block* prev;
} block_t;

static block_t* memory_head = NULL;
static block_t* fmemory_head = NULL;

void mm_init();
uint32_t mm_malloc(uint32_t size);
void mm_free(uint32_t addr);

void mm_init(){
    memory_head = (block_t*)get_ram_ptr(0x00);
    memory_head->size = RAM_SIZE / 2 - sizeof(block_t);
    memory_head->free = 1;
    memory_head->next = NULL;
    memory_head->prev = NULL;

    fmemory_head = (block_t*)get_ram_ptr(64 * 1024 * 1024);
    fmemory_head->size = RAM_SIZE / 2 - sizeof(block_t);
    fmemory_head->free = 1;
    fmemory_head->next = NULL;
    fmemory_head->prev = NULL;
}

uint32_t mm_malloc(uint32_t size){
    block_t* curr = memory_head;
    size = (size+7) & ~7;
    while(curr){
        if(curr->free && curr->size >= size){
            if(curr->size > size + sizeof(block_t) + 1024){
                block_t* new_block = (block_t*)((char*)curr + sizeof(block_t) + size);
                new_block->free = 1;
                new_block->size = curr->size - size - sizeof(block_t);
                new_block->next = curr->next;
                new_block->prev = curr;

                if(new_block->next != NULL) new_block->next->prev = new_block;

                curr->size = size;
                curr->next = new_block;
            }
            curr->free = 0;

            return (uint32_t)((uintptr_t)curr - (uintptr_t)get_ram_ptr(0x00) + sizeof(block_t));
        }
        curr = curr->next;
    }

    return 0;
}

void mm_free(uint32_t addr){
    if(addr == 0) return;
    block_t* memory = (block_t*)((char*)get_ram_ptr(addr) - sizeof(block_t));
    memory->free = 1;

    while(memory->prev != NULL && memory->prev->free == 1){
        memory = memory->prev;
    }

    while(memory->next != NULL && memory->next->free == 1){
        memory->size += memory->next->size + sizeof(block_t);
        memory->next = memory->next->next;

        if(memory->next != NULL) memory->next->prev = memory;
    }
}

uint32_t fmalloc(uint32_t size){
    block_t* curr = fmemory_head;
    size = (size+7) & ~7;
    while(curr){
        if(curr->free && curr->size >= size){
            if(curr->size > size + sizeof(block_t) + 1024){
                block_t* new_block = (block_t*)((char*)curr + sizeof(block_t) + size);
                new_block->free = 1;
                new_block->size = curr->size - size - sizeof(block_t);
                new_block->next = curr->next;
                new_block->prev = curr;

                if(new_block->next != NULL) new_block->next->prev = new_block;

                curr->size = size;
                curr->next = new_block;
            }
            curr->free = 0;

            return (uint32_t)((uintptr_t)curr - (uintptr_t)get_ram_ptr(0x00) + sizeof(block_t));
        }
        curr = curr->next;
    }

    return 0;
}

void ffree(uint32_t addr){
    if(addr == 0) return;
    block_t* memory = (block_t*)((char*)get_ram_ptr(addr) - sizeof(block_t));
    memory->free = 1;

    while(memory->prev != NULL && memory->prev->free == 1){
        memory = memory->prev;
    }

    while(memory->next != NULL && memory->next->free == 1){
        memory->size += memory->next->size + sizeof(block_t);
        memory->next = memory->next->next;

        if(memory->next != NULL) memory->next->prev = memory;
    }
}
#include "loader.h"
#include "process.h"
#include "syscall.h"
#include "ram.h"
#include "mm.h"
#include "vfs.h"
#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int exec(int argc, char** argv){
    if(argc != 2){
        printf("Usage: exec <elf_file>.elf\n\r");
    }
    lock_scheduler();
    char* elf_name = argv[1];
    int fd = vfs_open(elf_name);
    if (fd < 0) return -1;

    uint8_t* elf_raw = (uint8_t*)get_file_address(fd);
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)elf_raw;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Error: Not a valid ELF\n");
        vfs_close(fd);
        return -1;
    }

    Elf64_Phdr* phdr_table = (Elf64_Phdr*)(elf_raw + ehdr->e_phoff);
    Elf64_Addr base_vaddr = UINT64_MAX;
    Elf64_Addr top_vaddr  = 0;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *ph = &phdr_table[i];
        if (ph->p_type != PT_LOAD)
            continue;

        if(ph->p_memsz == 0)
            continue;

        if (ph->p_vaddr < base_vaddr)
            base_vaddr = ph->p_vaddr;

        if (ph->p_vaddr + ph->p_memsz > top_vaddr)
            top_vaddr = ph->p_vaddr + ph->p_memsz;
    }
    if (base_vaddr == UINT64_MAX) {
        printf("No PT_LOAD segments found");
        return -1;
    }


    size_t load_size = top_vaddr - base_vaddr;

    uint32_t vram = mm_malloc(load_size);
    void *physical_ram = get_ram_ptr(vram);

    // Important
    memset(physical_ram, 0, load_size);

    for (int i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr *ph = &phdr_table[i];
        if (ph->p_type != PT_LOAD)
            continue;

        uint8_t *dst = (uint8_t *)physical_ram + (ph->p_vaddr - base_vaddr);
        uint8_t *src = elf_raw + ph->p_offset;

        memcpy(dst, src, ph->p_filesz);

        // Zero bss
        if (ph->p_memsz > ph->p_filesz) {
            memset(dst + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
        }
    }

    uint32_t stack_top = mm_malloc(STACK_SIZE);

    void (*_start)(void) = (void (*)(void))((uint8_t *)physical_ram + (ehdr->e_entry - base_vaddr));


    vfs_close(fd);
    // printf("ELF base vaddr: 0x%lx\n", base_vaddr);
    // printf("ELF entry vaddr: 0x%lx\n", ehdr->e_entry);
    // printf("ELF entry ram:   %p\n", _start);

    k_spawn(_start, stack_top);
    unlock_scheduler();
    return 0;
}
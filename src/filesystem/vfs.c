#include "vfs.h"
#include "ram.h"
#include "cpu.h"
#include "mm.h"
#include "process.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

static inode_t inode_table[MAX_FILES];

void vfs_init(){
    memset(inode_table, 0, sizeof(inode_table));
    printf("[SUCCESS] FileSystem Initialized\n\r");
    bootstrap_vfs_from_host();
}

void bootstrap_vfs_from_host() {
    DIR *d;
    struct dirent *dir;
    // Open the folder on your laptop where the Makefile puts the ELFs
    d = opendir("src/user"); 

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            // Only process files ending in .elf
            if (strstr(dir->d_name, ".elf")) {
                char host_path[256];
                snprintf(host_path, sizeof(host_path), "src/user/%s", dir->d_name);

                // 1. Open file on your laptop
                FILE *f = fopen(host_path, "rb");
                if (!f) continue;

                // 2. Get the size
                fseek(f, 0, SEEK_END);
                long size = ftell(f);
                fseek(f, 0, SEEK_SET);

                // 3. Create the file entry in your VFS
                // This calls fmalloc internally to carve space from your 64MB
                if (vfs_create(dir->d_name, (int)size) == 0) {
                    
                    // 4. Use your VFS logic to write the bytes
                    int fd = vfs_open(dir->d_name);
                    if (fd != -1) {
                        void *temp_buf = malloc(size);
                        fread(temp_buf, 1, size, f);
                        
                        // Writes directly to your simulated RAM
                        vfs_write(fd, temp_buf, (int)size, 0);
                        
                        vfs_close(fd);
                        free(temp_buf);
                        printf("[LOADER] Loaded %s (%ld bytes) into VFS\n", dir->d_name, size);
                    }
                }
                fclose(f);
            }
        }
        closedir(d);
    }
}

int vfs_open(char* name){
    task_t* curr = get_current_task();
    int current_inode = -1;
    for(int inode = 0; inode < MAX_FILES; inode++){
        if(strcmp(inode_table[inode].file_name, name) == 0 && inode_table[inode].is_used){
            current_inode = inode;
            break;
        }
    }

    if(current_inode == -1){
        printf("[FATAL] File not found\n\r");
        return -1;
    }

    int fd = -1;
    for(int i = 0; i < 8; i++){
        if(!curr->fds[i].is_open) {
            fd = i;
            break;
        }
    }

    if(fd == -1){
        printf("[FATAL] Maximum files opened\n\r");
        return -1;
    }

    curr->fds[fd].inode_idx = current_inode;
    curr->fds[fd].offset = 0;
    curr->fds[fd].is_open = 1;

    return fd;
}

int vfs_close(int fd){
    task_t* curr = get_current_task();
    if(fd < 0 || fd > 7) return -1;
    curr->fds[fd].is_open = 0;
    return 1;
}

int vfs_read(int fd, void* buf, int n) {
    task_t* curr = get_current_task();

    if (fd < 0 || fd >= 8 || !curr->fds[fd].is_open) {
        printf("[FATAL] Invalid file descriptor\n");
        return -1;
    }
    
    int inode_idx = curr->fds[fd].inode_idx;
    uint32_t cursor = curr->fds[fd].offset;

    inode_t* file = &inode_table[inode_idx];

    void* src = (char*)get_ram_ptr(file->start_addr) + cursor;
    memcpy(buf,src,n);
    curr->fds[fd].offset += n;

    return n;
}

int vfs_write(int fd, void* info, int size, int append){
    task_t* curr = get_current_task();

    if (fd < 0 || fd >= 8 || !curr->fds[fd].is_open) {
        printf("[FATAL] Invalid file descriptor\n");
        return -1;
    }

    int inode_idx = curr->fds[fd].inode_idx;
    uint32_t cursor = curr->fds[fd].offset;

    inode_t* file = &inode_table[inode_idx];
    if(append) cursor = file->mused;

    void* file_writer = (char*)get_ram_ptr(file->start_addr) + cursor;
    if(file->size - cursor < size){
        printf("[FATAL] File does not have enough space\n\r");
        return -1;
    }
    memcpy(file_writer, info, size);
    curr->fds[fd].offset += size;

    if(curr->fds[fd].offset > file->mused){
        file->mused = curr->fds[fd].offset;
    }

    return size;
}

int vfs_lseek(int fd, int offset, int mode){
    task_t* curr = get_current_task();

    if (fd < 0 || fd >= 8 || !curr->fds[fd].is_open) return -1;

    inode_t* file = &inode_table[curr->fds[fd].inode_idx];

    if(mode == 0){
        if(offset < 0 || offset > file->size) {printf("[FATAL] Invalid file access\n\r"); return -1;}
        curr->fds[fd].offset = offset;
        if(offset > file->mused) curr->fds[fd].offset = file->mused;
    }
    else if(mode == 1){
        if(curr->fds[fd].offset + offset > file->size || curr->fds[fd].offset + offset < 0) {printf("[FATAL] Invalid file access\n\r"); return -1;}
        curr->fds[fd].offset += offset;
        if(curr->fds[fd].offset > file->mused) curr->fds[fd].offset = file->mused;
    }
    else if(mode == 2){
        if(offset > 0 || curr->fds[fd].offset + offset < 0) {printf("[FATAL] Invalid file access\n\r"); return -1;}
        curr->fds[fd].offset = file->mused + offset;
    }

    return curr->fds[fd].offset;
}

int vfs_create(char* file_name, int size){
    task_t* t = get_current_task();
    int current_inode = -1;
    for(int i=0; i < MAX_FILES; i++){
        if(inode_table[i].is_used == 0){
            current_inode = i;
            break;
        }
    }
    if(current_inode == -1){
        printf("[ERROR] Maximum files reached\n\r");
        return -1;
    }

    inode_table[current_inode].start_addr = fmalloc(size);
    if(inode_table[current_inode].start_addr == 0){
        printf("[FATAL] Unable to allocate memory to file: %d\n\r", current_inode);
        return -1;
    }
    strncpy(inode_table[current_inode].file_name, file_name, MAX_FILENAME - 1);//strlen(file_name));
    inode_table[current_inode].file_name[MAX_FILENAME - 1] = '\0';
    inode_table[current_inode].size = size;
    inode_table[current_inode].mused = 0;
    inode_table[current_inode].is_used = 1;

    int current_file = -1;
    for(int i=0; i<8; i++){
        if(!t->fds[i].is_open) {current_file = i; break;}
    }
    if(current_file == -1){
        printf("[ERROR] Maximum files opened for task");
    }

    t->fds[current_file].inode_idx = current_inode;
    t->fds[current_file].is_open = 1;
    t->fds[current_file].offset = 0;

    return 0;
}

int vfs_delete(char* name){
    for(int i=0; i<MAX_FILES; i++){
        if(inode_table[i].is_used && strcmp(inode_table[i].file_name, name) == 0){
            ffree(inode_table[i].start_addr);
            memset(&inode_table[i], 0, sizeof(inode_t));

            printf("[SYSTEM] File '%s' deleted successfully\n\r",name);
            return 0;
        }
    }

    printf("[SYSTEM] File '%s' not found\n\r", name);
    return -1;
}

uint8_t* get_file_address(int fd){
    task_t* curr = get_current_task();
    int inode_idx = curr->fds[fd].inode_idx;

    inode_t* file = &inode_table[inode_idx];

    return (uint8_t*)((char*)get_ram_ptr(file->start_addr));
}
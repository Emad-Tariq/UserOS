#include "syscall.h"
#include "process.h"
#include "vfs.h"

int f_open(char* name){
    lock_scheduler();
    int success = vfs_open(name);
    unlock_scheduler();
    return success;
}

int f_close(int fd){
    lock_scheduler();
    int success = vfs_close(fd);
    unlock_scheduler();
    return success;
}

int f_read(int fd, void* buf, int size){
    lock_scheduler();
    int success = vfs_read(fd,buf,size);
    unlock_scheduler();
    return success;
}

int f_write(int fd, void* info, int size, int append){
    lock_scheduler();
    int success = vfs_write(fd, info, size, append);
    unlock_scheduler();
    return success;
}

int f_lseek(int fd, int offset, int mode){
    lock_scheduler();
    if(mode < 0 || mode > 2){printf("[FATAL] Invalid mode\n\r"); return -1;}
    int success = vfs_lseek(fd, offset, mode);
    unlock_scheduler();
    return success;
}

int f_create(char* name, int size){
    lock_scheduler();
    int success = vfs_create(name, size);
    unlock_scheduler();
    return success;
}

int f_delete(char* name){
    lock_scheduler();
    int success = vfs_delete(name);
    unlock_scheduler();
    return success;
}

uint8_t* f_getaddr(int fd){
    return get_file_address(fd);
}

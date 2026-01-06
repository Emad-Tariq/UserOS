#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

int f_open(char* name);
int f_close(int fd);
int f_read(int fd, void* buf, int size);
int f_write(int fd, void* info, int size, int append);
int f_lseek(int fd, int offset, int mode);
int f_create(char* name, int size);
int f_delete(char* name);
uint8_t* f_getaddr(int fd);


#endif
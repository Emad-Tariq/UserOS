#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define MAX_FILES 32
#define MAX_FILENAME 16

typedef struct {
    char file_name[MAX_FILENAME];
    uint32_t start_addr;
    size_t size;
    size_t mused;
    int is_used;
} inode_t;

void vfs_init();
int vfs_open(char* name);
int vfs_close(int fd);
int vfs_read(int fd, void* buf, int n);
int vfs_write(int fd, void* info, int size, int append);
int vfs_lseek(int fd, int offset, int mode);
int vfs_create(char* file_name, int size);
int vfs_delete(char* name);
uint8_t* get_file_address(int fd);


#endif
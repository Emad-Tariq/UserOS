#ifndef CPU_H
#define CPU_H

#include <ucontext.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE (64 * 1024)
#define MAX_TASKS 16

typedef enum{
    STATE_EMPTY,
    STATE_READY,
    STATE_RUNNING,
    STATE_WAITING,
    STATE_FINISHED,
}task_state_t;

// In cpu.h
typedef struct {
    int inode_idx;
    uint32_t offset;
    int is_open;
} file_descriptor_t;

typedef struct{
    ucontext_t context;
    int pid;
    uint32_t stack_base;
    uint32_t stack_size;
    task_state_t state;
    file_descriptor_t fds[8];
}__attribute__((aligned(16))) task_t;

typedef struct {
    task_t tasks[MAX_TASKS];
    int current_task_id;
    int num_tasks;
} process_table_t;


void cpu_timer_init(int ms);
void cpu_create_task(task_t* task, void (*func)(void), uint32_t stack_addr);
void cpu_switch(task_t* old_task, task_t* new_task);

#endif
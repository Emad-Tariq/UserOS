#ifndef PROCESS_H
#define PROCESS_H

#include "cpu.h"

extern process_table_t p_table;


void lock_scheduler();
void unlock_scheduler();

void process_init();
void task_wrapper(uintptr_t entry_raw);
int get_next_ready_task(int current_pid);
void k_yield();
void timer_handler(int sig, siginfo_t* info, void* secret);
int k_spawn(void (*func)(void), uint32_t stack);
void k_exit(void);
task_t* get_current_task();

#endif
#include "ram.h"
#include "cpu.h"
#include "process.h"
#include "mm.h"
#include "vfs.h"
#include "shell.h"
#include <stdio.h>

// A global to save the "Boot/Kernel" state
task_t kernel_context;

void task_a();
void task_b();
void task_c();

void shell() {
    printf("\n--- Welcome to the Pseudo-OS Shell ---\n");
    printf("Shell is running with its own stack in PRAM!\n");
    shell_task();
}

void task_a() {
    printf("Task A is working...\n");
    for(volatile int i=0; i<100000000; i++); // Busy work
    printf("Task A is over...\n\r");
}

void task_b() {
    while(1) {
        printf("Task B is polite, it does not have alot of work!\n");
        for(volatile int i=0; i < 10000000; i++);
    }
}

void task_c(){
    while(1) {
        printf("Task C is dumb!\n");
        for(volatile int i=0; i < 10000000; i++);
    }
}

int main() {
    printf("----------BOOT UP----------\n");

    // 1. Hardware & System Init
    ram_init();
    mm_init();
    process_init();
    vfs_init();
    cpu_timer_init(100);


    // 2. Create the first process (Shell)
    int shell_tid = k_spawn(shell, mm_malloc(STACK_SIZE));

    if (shell_tid != -1) {
        printf("[BOOT] Switching to Shell (TID %d)...\n", shell_tid);
        p_table.current_task_id = shell_tid;
        p_table.tasks[shell_tid].state = STATE_RUNNING;
        
        // JUMP!
        cpu_switch(&kernel_context, &p_table.tasks[shell_tid]);
    }

    return 0; // Should never reach here if shell runs
}
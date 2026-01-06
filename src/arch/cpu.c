#include "cpu.h"
#include "ram.h"
#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>


void cpu_timer_init(int ms){
    struct sigaction sa;
    struct itimerval timer;

    sa.sa_sigaction = timer_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = ms * 1000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = ms * 1000;

    setitimer(ITIMER_REAL, &timer, NULL);
    printf("[SUCCESS] Timer interrupt set to %d ms\n\r",ms);
}

void cpu_create_task(task_t* task, void (*func)(void), uint32_t stack_addr){
    if(getcontext(&task->context) == -1){
        perror("[FATAL] Failed to get context\n\r");
        exit(1);
    }

    void* physical_stack_addr = get_ram_ptr(stack_addr);
    if(!physical_stack_addr){
        fprintf(stderr, "[FATAL] Stack allocation failed\n\r");
        exit(1);
    }

    sigemptyset(&task->context.uc_sigmask);
    
    //3. FORCE 16-byte alignment
    uintptr_t stack_ptr = (uintptr_t)physical_stack_addr;
    uintptr_t aligned_ptr = (stack_ptr + 15) & ~15;

    task->context.uc_stack.ss_sp = (void*)aligned_ptr;
    task->context.uc_stack.ss_size = (STACK_SIZE - 64) & ~15;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = NULL;

    makecontext(&task->context, (void(*)())task_wrapper, 1, (uintptr_t)func);


    task->stack_base = stack_addr;
    task->stack_size = STACK_SIZE;

    printf("[KERNEL] Spawned task at location: 0x%08lx, ", aligned_ptr);
}

void cpu_switch(task_t* old_task, task_t* new_task){
    swapcontext(&old_task->context, &new_task->context);
}
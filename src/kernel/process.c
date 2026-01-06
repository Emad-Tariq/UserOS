#include "process.h"
#include "mm.h"
#include "ram.h"
#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

process_table_t p_table;

void safe_debug(const char* msg) {
    write(STDOUT_FILENO, msg, strlen(msg));
}

void task_wrapper(uintptr_t entry_raw) {
    void (*entry)(void) = (void (*)(void))entry_raw;
    entry();
    k_exit();
}



void lock_scheduler(){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_BLOCK, &set, NULL);
}

void unlock_scheduler(){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
}

void process_init(){
    p_table.num_tasks = 0;
    p_table.current_task_id = 0;

    for(int i=0; i<MAX_TASKS; i++){
        p_table.tasks[i].pid = i;
        p_table.tasks[i].state = STATE_EMPTY;
        p_table.tasks[i].stack_base = 0;
    }

    printf("[SUCCESS] Process Table initialized\n\r");
}

int get_next_ready_task(int current_pid){
    for(int pid = 1; pid <= MAX_TASKS; pid++){
        int i = (pid + current_pid) % MAX_TASKS;
        if(p_table.tasks[i].state == STATE_READY && i != current_pid){
            return i;
        }
    }

    return -1;
}

void k_yield() {
    lock_scheduler();
    int current_pid = p_table.current_task_id;
    int next_pid = get_next_ready_task(current_pid);

    if(next_pid == -1) {
        //printf("[FATAL] No tasks to run\n\r");
        unlock_scheduler();
        return;
    }

    //printf("[KERNEL] Task is yielding\n\r");

    p_table.tasks[current_pid].state = STATE_READY;
    p_table.tasks[next_pid].state = STATE_RUNNING;
    p_table.current_task_id = next_pid;

    //unlock_scheduler();
    cpu_switch(&p_table.tasks[current_pid], &p_table.tasks[next_pid]);
    unlock_scheduler();
}

// In process.c
void timer_handler(int sig, siginfo_t* info, void* secret) {
    //safe_debug("[SYSTEM] Timer Interrupt\n\r");
    (void)info;
    if(sig != SIGALRM){
        printf("[FATAL] Timer Interrupt failure\n\r");
        return;
    }
    ucontext_t* old_context = (ucontext_t*)secret;
    int current_pid = p_table.current_task_id;
    if(current_pid == -1) return;

    memcpy(&p_table.tasks[current_pid].context, old_context, sizeof(ucontext_t));

    k_yield();
}

int k_spawn(void (*func)(void), uint32_t stack){
    lock_scheduler();
    if(p_table.num_tasks >= MAX_TASKS) return -1;

    int pid = -1;
    for(int i=0; i<MAX_TASKS; i++){
        if(p_table.tasks[i].state == STATE_EMPTY){
            pid = i;
            break;
        }
    }

    // uint32_t stack = mm_malloc(STACK_SIZE);
    if(stack == 0){
        printf("[SYSTEM] Memory unavailable for process: %d\n\r",pid);
        unlock_scheduler();
    }
    cpu_create_task(&p_table.tasks[pid], func, stack);

    p_table.tasks[pid].state = STATE_READY;
    p_table.num_tasks++;

    printf("PID: %d\n\r",pid);
    unlock_scheduler();
    return pid;
}

task_t* get_current_task(void){
    return &p_table.tasks[p_table.current_task_id];
}

void k_exit(void){
    lock_scheduler();
    int pid = p_table.current_task_id;
    task_t* curr = get_current_task();
    for (int i = 0; i < 8; i++) {
        if (curr->fds[i].is_open) {
            // Log it so you can see if your tasks are being "messy"
            printf("[KERNEL] Cleaning up abandoned FD %d for PID %d\n", i, curr->pid);
            curr->fds[i].is_open = 0;
            curr->fds[i].offset = 0;
            // If you have a 'reference count' on your inodes, decrement it here
        }
    }
    mm_free(p_table.tasks[pid].stack_base);

    p_table.tasks[pid].state = STATE_EMPTY;
    p_table.num_tasks--;

    int nextpid = get_next_ready_task(pid);
    if(nextpid == -1) {
        safe_debug("[FATAL] No tasks to run\n\r");
        exit(0);
    }

    p_table.current_task_id = nextpid;
    p_table.tasks[nextpid].state = STATE_RUNNING;

    setcontext(&p_table.tasks[nextpid].context);
}
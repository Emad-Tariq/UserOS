#include "process.h"
#include <stdio.h>

int ps(int argc, char** argv){
    printf("\nPID | STATE      | STACK BASE | Capacity\n\r");
    printf("------------------------------------------\n\r");

    lock_scheduler();

    for(int i=0; i < MAX_TASKS; i++){
        task_t* t = &p_table.tasks[i];

        if(t->state == STATE_EMPTY) continue;

        const char* state_str;
        switch(t->state) {
            case STATE_READY:    state_str = "READY"; break;
            case STATE_RUNNING:  state_str = "RUNNING"; break;
            case STATE_WAITING:  state_str = "WAITING"; break;
            case STATE_FINISHED: state_str = "FINISHED"; break;
            default:             state_str = "UNKNOWN"; break;
        }

        // Highlight the currently running task with an asterisk
        char current_marker = (i == p_table.current_task_id) ? '*' : ' ';

        printf("%c%-2d | %-10s | 0x%08x | %d KB\n", 
               current_marker,
               t->pid, 
               state_str, 
               t->stack_base,
               STACK_SIZE / 1024
            );
    }

    unlock_scheduler();
    printf("------------------------------------------\n\r");
    printf("Total active tasks: %d\n\r", p_table.num_tasks);
}
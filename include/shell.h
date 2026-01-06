#ifndef SHELL_H
#define SHELL_H

#define MAX_ARGS 16

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "syscall.h"
#include "process.h"

typedef int (*cmd_handler_t)(int argc, char** argv);

typedef struct{
    const char* name;
    cmd_handler_t handler;
    const char* info;
} shell_command_t;

void shell_task();

#endif
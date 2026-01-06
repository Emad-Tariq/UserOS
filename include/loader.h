#ifndef LOADER_H
#define LOADER_H

#include <elf.h>
#include "syscall.h"

int exec(int argc, char** argv);

#endif
#include "shell.h"
#include "app.h"
#include "loader.h"
#include "process.h"
#include <errno.h>

static shell_command_t commands[] = {
    {"exec", exec, "The loader function responsible for execution of tasks"},
    {"ls", ls, "List all files in the system"},
    {"cat", cat, "Display content of a file"},
    {"touch", touch, "Create a new empty file"},
    {"ps", ps, "Lists all process information"},
    {"help", help, "Presents this list"}
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(shell_command_t))


void shell_task(){
    char input[128];
    char* argv[MAX_ARGS];
    
    while(1){
        printf("Emad:~$ ");
        fflush(stdout);

        char* status = NULL;
        while (status == NULL) {
            status = fgets(input, sizeof(input), stdin);
            if (status == NULL) {
                if (feof(stdin)) return; // Exit on Ctrl+D
                if (errno == EINTR) {
                    clearerr(stdin); // Clear the 'interrupted' flag
                    continue;        // Try reading again without re-printing prompt
                }
                break;
            }
        }

        if (status == NULL) continue;
        input[strcspn(input, "\n")] = '\0';

        char* token = strtok(input, " ");
        int argc = 0;
        while(token != NULL){
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }

        if(argc == 0) continue;

        int found = 0;
        for(uint i=0; i<NUM_COMMANDS; i++){
            //printf("%s\n\r",commands[i].name);
            if(strcmp(argv[0], commands[i].name) == 0){
                commands[i].handler(argc,argv);
                found = 1;
                break;
            }
        }

        if(!found){
            printf("Unknown command: %s\n\r",argv[0]);
        }
    }
}
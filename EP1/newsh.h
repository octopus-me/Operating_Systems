#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <string.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <pwd.h>

#include <sys/utsname.h>

void execute_shell_cicles();

int tokenizer(char comando[], char *args[]);
int select_arguments(char *args[], char *new_args[]);

void execute_ps(char *args[], char *new_args[], char comando[]);
void execute_ls(char *args[], char *new_args[], char comando[]);
void execute_ep1(char *args[], char *new_args[], char comando[]);
void execute_cd(char *args[]);
void execute_rm(char *args[]);
void execute_uname(char *args[]);

void executable_or_syscall(char *args[], char comando[]);
void verify_executable(char *args[], char comando[]);

char *get_username();
char *get_current_time();

void free_memory(char *args[], int arg_count)

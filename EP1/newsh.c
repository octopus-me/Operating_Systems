#include "newsh.h"

#define MAX_STRING_LENGTH 1024

int main() {
    
    execute_shell_cicles();

    return 0;
}

void execute_shell_cicles(){

    using_history();

    int k = 1;
    while (k==1) {
        k++;

        char *username = get_username();

        char *current_time = get_current_time();

        printf("%s [%s] : ", username , current_time);

        char *comando;

        comando = readline("");

        if (comando && *comando){
            add_history(comando);
        }

        comando[strcspn(comando, "\n")] = '\0';

        char *args[MAX_STRING_LENGTH];

        int arg_count;

        arg_count = tokenizer(comando, args);

        executable_or_syscall(args,comando);
        
        free_memory(args,arg_count);
        free(current_time);
        free(comando);

        arg_count = 0; // Reset arg_count for the next iteration
    }

}

char *get_current_time(){
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char* current_time = (char*)malloc(MAX_STRING_LENGTH*sizeof(char));
    strftime(current_time,MAX_STRING_LENGTH,"%H:%M:%S", timeinfo);
    return current_time;
}

char *get_username(){
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    return pw->pw_name;
}

void executable_or_syscall(char *args[], char comando[]){

    if (strncmp(args[0], "cd", 2) == 0) {
        execute_cd(args);
    } else if (strncmp(args[0],"rm", 2) == 0) {
        execute_rm(args);
    } else if (strncmp(args[0], "uname", 5)==0){
        execute_uname(args);
    } else {
        verify_executable(args,comando);
    }
}

void execute_cd(char *args[]){
    chdir(args[1]);
}

void execute_rm(char *args[]){
    remove(args[1]);
}

void execute_uname(char *args[]){
    struct utsname uts;
    uname(&uts);
    printf("%s - ", uts.sysname);
    printf("%s - ", uts.nodename);
    printf("%s - ", uts.release);
    printf("%s - ", uts.version);
    printf("%s - \n", uts.machine);
}

void verify_executable(char *args[], char comando[]){

    char *new_args[MAX_STRING_LENGTH - 1];
    int size = select_arguments(args, new_args);

    if (strncmp(args[0], "/bin/ps", 7) == 0 && strncmp(args[1], "a", 1)== 0){
        execute_ps(args, new_args, comando);
    } else if(strncmp(args[0], "/bin/ls", 7) == 0 && strncmp(args[1], "--color=never", 13) == 0 && strncmp(args[2], "-1t", 3) == 0) {
        execute_ls(args, new_args, comando);
    } else if(strncmp(args[0],"./ep1",5)==0){
        printf("executar ep1 \n");
        execute_ep1(args,new_args,comando);
    }

    free_memory(new_args, size);
    
}

void execute_ps(char *args[], char *new_args[], char comando[]){
    pid_t pid = fork();
    if (pid != 0) {
        waitpid(-1,NULL,0);
    } else {
        execve(args[0], new_args, NULL);
    }
}

void execute_ls(char *args[], char *new_args[], char comando[]){
    pid_t pid = fork();
    if (pid != 0){
        waitpid(-1,NULL, 0);
    } else {
        execve(args[0],new_args, NULL);
    }
}

void execute_ep1(char *args[], char *new_args[], char comando[]){
    pid_t pid = fork();
    if (pid != 0){
        waitpid(-1, NULL, 0);
    } else {
        execve(args[0], new_args, NULL);
    }
}

int select_arguments(char *args[], char *new_args[]){
    int size = 0;
    while(args[size] != NULL){
        size += 1;
    }

    for (int i = 1; i< size; i++){
        new_args[i-1] = strdup(args[i]);
    }
    new_args[size-1] = NULL;
    return size-1;
}

int tokenizer(char comando[], char *args[]){

    int arg_count = 0;

    char *token = strtok(comando, " ");
    while (token != NULL) {
        args[arg_count++] = strdup(token); // allocate memory for each token
        token = strtok(NULL, " ");
    }
    return arg_count;
}

void free_memory(char *args[], int arg_count){
    for (int i = 0; i < arg_count; i++) {
        free(args[i]);
    }
}

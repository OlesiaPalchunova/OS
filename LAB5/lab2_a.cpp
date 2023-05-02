/*
Чтобы сделать дочерний процесс зомби, необходимо не вызывать
функцию wait() в родительском процессе после завершения 
дочернего процесса. В этом случае дочерний процесс будет 
оставаться в состоянии "зомби" до тех пор, пока родительский 
процесс не вызовет wait() или waitpid() для получения кода завершения 
дочернего процесса.

Зомби-процессы не являются проблемой в смысле нагрузки на систему 
или потребления ресурсов. Однако, если зомби-процессов становится 
слишком много, это может привести к исчерпанию ресурсов ядра (например, памяти ядра). 

Кроме того, зомби-процессы могут оставаться в списке процессов 
(например, при вызове ps) и мешать администраторам системы 
в отслеживании активных процессов.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    int local_var = 10;
    pid_t pid;

    printf("Address of local_var: %p\n", &local_var);
    printf("Value of local_var: %d\n", local_var);
    printf("PID: %d\n", getpid());

    pid = fork();

    if (pid == -1) {
        printf("Failed to fork\n");
        exit(1);
    } else if (pid == 0) {
        printf("Child PID: %d\n", getpid());
        printf("Parent PID: %d\n", getppid());
        printf("Address of local_var: %p\n", &local_var);
        printf("Value of local_var: %d\n", local_var);
        local_var = 20;
        printf("New value of local_var: %d\n", local_var);
        exit(5);
    } else {
        sleep(5);
        printf("Parent PID: %d\n", getpid());
        printf("Waiting for child process to finish...\n");
        // Здесь нет вызова wait()
    }

    return 0;
}

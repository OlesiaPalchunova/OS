/*
Родительский процесс не может стать зомби-процессом, 
поскольку он должен вызвать функцию wait() или waitpid() 
для получения кода завершения дочернего процесса. 

Однако, если родительский процесс завершится, а дочерний 
процесс еще не завершился, то дочерний процесс будет 
унаследовать процесс-родитель, который станет процессом-посредником (init). 
В этом случае дочерний процесс не будет иметь родительского процесса, 
и его код завершения будет сохранен в системе до тех пор, 
пока он не будет прочитан другим процессом с помощью функции wait() или waitpid(). 
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
        sleep(10);
        printf("Child process is now terminated\n");
        exit(5);
    } else {
        printf("Parent PID: %d\n", getpid());
        printf("Waiting for child process to finish...\n");
        sleep(5);
        printf("Parent process is now terminated\n");
        // Родительский процесс завершается без вызова wait()
    }

    return 0;
}

/*
Кода завершения не будет, если дочерний процесс станет зомби-процессом.
Чтобы избежать этой ситуации, родительский процесс должен вызвать функцию 
wait() или waitpid() для каждого созданного им дочернего процесса, чтобы 
получить его код завершения и удалить запись о процессе из таблицы процессов.

Значения переменных в родительском процессе не изменились, потому что 
созданный дочерний процесс является отдельным процессом со своим собственным 
адресным пространством, включая собственный стек и собственные значения переменных. 

Картинка, которая показывает, как появляются Zombie-процессы
https://static.javatpoint.com/operating-system/images/what-is-zombie-process.png
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int global_var = 10;

int main() {
    int local_var = 20;

    printf("global_var: address=%p, value=%d\n", &global_var, global_var); // value=10
    printf("local_var: address=%p, value=%d\n", &local_var, local_var); // value=20

    printf("pid: %d\n", getpid());

    sleep(10);

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) { // дочерний процесс

        sleep(5);

        printf("child pid: %d, parent pid: %d\n", getpid(), getppid());

        printf("global_var: address=%p, value=%d\n", &global_var, global_var); // value=10
        printf("local_var: address=%p, value=%d\n", &local_var, local_var); // value=20

        global_var = 30;
        local_var = 40;

        printf("global_var: %d, local_var: %d\n", global_var, local_var); // global_var: 30, local_var: 40

        exit(5);
    } else {

        sleep(10);

        printf("global_var: %d, local_var: %d\n", global_var, local_var); // global_var: 10, local_var: 20

        sleep(30);

        int status;
        pid_t child_pid = wait(&status);

        if (child_pid == -1) {
            perror("wait");
            exit(1);
        } else {
            if (WIFEXITED(status)) {
                printf("child process exited with code %d\n", WEXITSTATUS(status));
            } else {
                printf("child process terminated abnormally\n");
            }
        }
    }

    return 0;
}

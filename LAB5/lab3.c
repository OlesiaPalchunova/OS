/*
Удобно наблюдать за файлом через команду hexdump
*/

#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
 
#define STACK_SIZE (1024 * 1024)
#define FILE_NAME "file.txt"

// Вторая функция должна выделять на стеке массив со строкой “hello world” и рекурсивно вызывать себя
int second_function(int i) {
    if (i == 10) {
        return 0;
    }
    char hello_world[] = "hello world\n";
    return second_function(++i);
}

// Первая функция - точка входа для нового процесса
int first_function(void *arg) {
    second_function(0);
    return 0;
}
 
int main() {
    // открываем файл
    int fd = open(FILE_NAME, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    
    // Изменям размер файла до размера STACK_SIZE
    if (ftruncate(fd, STACK_SIZE) == -1) {
        perror("ftruncate");
        close(fd);
        return 1;
    }

    // создаем новый регион памяти на стеке для нового процесса
    void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (stack == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
 
    // создаем нового процесс, который вызывает функцию
    pid_t child_pid = clone(first_function, stack + STACK_SIZE, CLONE_VFORK | SIGCHLD, NULL);
    if (child_pid == -1) {
        perror("clone");
        munmap(stack, STACK_SIZE);
        close(fd);
        return 1;
    }
 
    int status;
    waitpid(child_pid, &status, 0);
 
    munmap(stack, STACK_SIZE);

    close(fd);
    return 0;
}

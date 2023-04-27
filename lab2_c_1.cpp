/*
В случае Стэка указателель будет иддти вверх, то есть второе число в его размере 
будет становиться меньше.

Куча наоборот расширяется вниз
*/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

int i = 0;

void Stack() {
    int buffer[4096];

    printf("%d: %d\n", i++, getpid());
    usleep(100000);
    Stack();
}

void Heap(){
    long size = 0;
    const int block = 1024*10;

    while (1) {
        malloc(block);
        size += block;

        printf("pid %d; size %ld\n", getpid(), size);

        usleep(100000);
        if (size > block*50){
            mmap(NULL, 1024*10, PROT_NONE, MAP_ANONYMOUS, -1, 0);
        }
    }

}

int main() {
    printf("pid: %d\n", getpid());
    sleep(10);
    Stack();
    Heap();
    return 0;
}

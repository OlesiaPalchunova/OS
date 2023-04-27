/*
Программа из-за (1) и (2) выдает:
Trying to read from protected region:
Segmentation fault (core dumped)

Если добавить signal(SIGSEGV, SIG_IGN), то процесс не будет прерываться и завершаться при возникновении ошибки доступа к памяти.
Он будет продолжать работу
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <mutex>
#include <csignal>

int main() {
    printf("pid: %d\n", getpid());

    sleep(10);

    int page_size = getpagesize();
    char *ptr = (char*)mmap(NULL, 10 * page_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    printf("Created new memory region of size %d\n", 10 * page_size);
    sleep(10);

    int result = mprotect(ptr + 3 * page_size, 3 * page_size, PROT_NONE);
    if (result == -1) {
        perror("mprotect");
    }

    char *p = ptr;
    printf("Trying to read from protected region:\n");
    printf("%d\n", p[4 * page_size]);  // (1)

    sleep(10);


    result = mprotect(ptr + 4 * page_size, 2 * page_size, PROT_READ);
    if (result == -1) {
        perror("mprotect");
    }

    printf("Wrote to protected region\n");
    //signal(SIGSEGV, SIG_IGN);
    p[5 * page_size] = 42; // (2)

    sleep(10);


    result = munmap(ptr + 3 * page_size, 3 * page_size);
    if (result == -1) {
        perror("munmap");
    }

    printf("End\n");
    return 0;
}

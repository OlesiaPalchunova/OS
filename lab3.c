#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

void *heap_start;
size_t heap_size;

typedef struct free_block {
    size_t size;
    struct free_block* next;
} free_block;

static free_block free_block_list_head = { 0, 0 };  //список свободных блоков

static const size_t align_to = 16;

void* my_malloc(size_t size) {
    size = (size + sizeof(free_block) + (align_to - 1)) & ~ (align_to - 1); // округляем до степени двойки в большую сторону
    free_block* block = free_block_list_head.next;
    free_block** head = &(free_block_list_head.next);
    while (block != 0) {
        if (block->size >= size) {
            *head = block->next;
            return ((char*)block) + sizeof(free_block); //нашли блок подходящего размера
        }
        head = &(block->next);
        block = block->next;
    }

    block = (free_block*)sbrk(size); // увеличиваем размер кучи
    block->size = size;

    return ((char*)block) + sizeof(free_block);
}

void my_free(void* ptr) {
    free_block* block = (free_block*)(((char*)ptr) - sizeof(free_block ));
    block->next = free_block_list_head.next;
    free_block_list_head.next = block; //добавляем блок в список свободных блоков
}

int main() {

    heap_size = PAGE_SIZE * 10;
    heap_start = mmap(NULL, heap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (heap_start == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    void *p1 = my_malloc(100);
    printf("p1 = %p\n", p1);
    void *p2 = my_malloc(200);
    printf("p2 = %p\n", p2);
    void *p3 = my_malloc(300);
    printf("p3 = %p\n", p3);

    my_free(p1);
    my_free(p2);
    my_free(p3);

    munmap(heap_start, heap_size);
    return 0;
}

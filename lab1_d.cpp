/*
Создаем переменную в функции и возвращаем ее адрес.
Программа выводит: Segmentation fault (core dumped)
Это происходит из-за того, что переменная будет уничтожена после завершения работы функции
*/

#include <stdio.h>
#include <stdlib.h>

int* create_and_init_local_variable() {
    int x = 42;
    return &x;
}

int main() {
    int* p = create_and_init_local_variable();
    printf("%dn", *p);
    return 0;
}

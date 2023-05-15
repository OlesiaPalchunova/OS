#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <cerrno>

#define PORT 8080
#define MAX_CLIENTS 10

int main(int argc, char const *argv[]) {
    int server_fd, max_fd, new_socket, activity, client_sockets[MAX_CLIENTS], i, valread;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[1024] = {0};
    fd_set read_fds;

    // создаем TCP-серверный сокет
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // задаем параметры адреса сервера
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // связываем серверный сокет с адресом
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // начинаем прослушивание входящих соединений
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // инициализируем массив дескрипторов клиентских сокетов
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // выводим сообщение о запуске сервера
    printf("Server started on port %d\n", PORT);

    while (1) {
        // очищаем набор дескрипторов для чтения
        FD_ZERO(&read_fds);

        // добавляем серверный сокет в набор дескрипторов для чтения
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd;

        // добавляем клиентские сокеты в набор дескрипторов для чтения
        for (i = 0; i < MAX_CLIENTS; i++) {
            int client_socket = client_sockets[i];

            if (client_socket > 0) {
                FD_SET(client_socket, &read_fds);
            }

            if (client_socket > max_fd) {
                max_fd = client_socket;
            }
        }

        // используем функцию select() для ожидания событий на дескрипторах
        activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        // если произошло событие на серверном сокете, значит есть новое входящее соединение
        if (FD_ISSET(server_fd, &read_fds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }

            // выводим сообщение о новом соединении
            printf("New connection, socket fd is %d, ip is : %s, port : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // добавляем новый клиентский сокет в массив
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    break;
                }
            }
        }

        // обрабатываем события на клиентских сокетах
        for (i = 0; i < MAX_CLIENTS; i++) {
            int client_socket = client_sockets[i];

            if (FD_ISSET(client_socket, &read_fds)) {
                // читаем данные от клиента
                memset(buffer, 0, sizeof(buffer));
                if ((valread = read(client_socket, buffer, 1024)) == 0) {
                    // клиент отключился, удаляем его сокет из массива
                    close(client_socket);
                    client_sockets[i] = 0;
                } else {
                    // пересылаем данные обратно клиенту
                    send(client_socket, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    return 0;
}

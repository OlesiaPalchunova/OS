#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main() {
    // IP-адрес и порт для прослушивания
    const char* ip = "127.0.0.1";
    const int port = 8080;

    // Создание TCP-сокета
    int server_socket = socket(AF_INET, SOCK_STREAM, 0); // SOCK_STREAM используются для протоколов, которые гарантируют доставку и упорядочивание передаваемых данных
    if (server_socket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Настройка адреса сервера
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    // преобразуем IP-адрес (ip) из текстового представления в двоичное представление и записываем в server_address.sin_addr
    inet_pton(AF_INET, ip, &server_address.sin_addr);

    // Привязка сокета к адресу сервера
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_socket);
        return 1;
    }

    // Начало прослушивания входящих соединений
    if (listen(server_socket, 5) < 0) { // этот сокет не принимает данные, он принимает соединения, 5 - число клиентов одновременно 
        std::cerr << "Error listening for connections" << std::endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Server listening on " << ip << ":" << port << std::endl;

    while (true) {
        // Ожидание нового клиента
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        // функция, которая принимает входящее соединение на серверном сокете и создает новый сокет для общения с клиентом.
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::cout << "New client connected: " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;

        // Создание нового процесса для клиента
        pid_t pid = fork();
        if (pid == 0) {
            // Дочерний процесс: чтение данных от клиента и отправка их обратно
            while (true) {
                char buffer[1024];
                ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
                if (bytes_received <= 0) {
                    break;
                }
                send(client_socket, buffer, bytes_received, 0);
            }

            // Закрытие сокета клиента и завершение дочернего процесса
            close(client_socket);
            std::cout << "Client disconnected: " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << std::endl;
            return 0;
        } else if (pid < 0) {
            std::cerr << "Error creating child process" << std::endl;
            close(client_socket);
        } else {
            // Родительский процесс: закрытие сокета клиента и продолжение прослушивания
            close(client_socket);
        }
    }

    // Закрытие серверного сокета
    close(server_socket);

    return 0;
}

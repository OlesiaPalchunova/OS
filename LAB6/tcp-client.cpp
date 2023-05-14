#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080

int main(int argc, char const *argv[]) {
    // IP-адрес и порт сервера
    const char* ip = "127.0.0.1";
    const int port = 8080;

    // Создание TCP-сокета
    int sock = 0;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }
    // установление соединения с сервером, который слушает определенный IP-адрес и порт
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    std::cout << "Enter message: ";
    std::cin.getline(buffer, 1024); // считываем сообщение
    send(sock, buffer, strlen(buffer), 0); // отправляем его
    std::cout << "Message sent" << std::endl;

    memset(buffer, 0, sizeof(buffer));
    int valread = read(sock, buffer, 1024); // принимаем мообщение в буфер
    std::cout << "Received message: " << buffer << std::endl;
    return 0;
}

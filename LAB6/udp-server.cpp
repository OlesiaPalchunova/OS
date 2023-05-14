#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int PORT = 8080; // номер порта
const int MAX_BUFFER_SIZE = 1024;

int main() {
    // создание сокета
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0); // SOCK_DGRAM - использование протокола UDP
    if (server_socket < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // заполняем данные об адресе клиента
    struct sockaddr_in server_address, client_address;
    std::memset(&server_address, 0, sizeof(server_address)); // очистка (зануление) структуры
    server_address.sin_family = AF_INET; // используется сетевой протокол IPv4
    server_address.sin_addr.s_addr = INADDR_ANY; // сервер слушает все интерфейсы
    server_address.sin_port = htons(PORT); // преобразовываем порядок байтов в сетевой порядок и присваиваем номер порта

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) { // связываем сокет с адресом сервера
        std::cerr << "Failed to bind socket\n";
        return 1;
    }

    char buffer[MAX_BUFFER_SIZE];
    int client_address_length = sizeof(client_address);

    while (true) {
        std::memset(buffer, 0, MAX_BUFFER_SIZE);
    
        // принимаем данные из сокета и записываем в буффер
        // server_address и server_address_length используются для получения информации об отправителе сообщения, которую можно использовать для отправки ответа
        int received_bytes = recvfrom(server_socket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&client_address, (socklen_t*)&client_address_length);
        if (received_bytes < 0) {
            std::cerr << "Failed to receive data\n";
            continue;
        }

        // выводим полученные данные на экран
        std::cout << "Received " << received_bytes << " bytes from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << "\n";
        std::cout << "Data: " << buffer << "\n";

        // отправляем данные
        int sent_bytes = sendto(server_socket, buffer, received_bytes, 0, (struct sockaddr*)&client_address, client_address_length);
        if (sent_bytes < 0) {
            std::cerr << "Failed to send data\n";
            continue;
        }
    }

    close(server_socket);
    return 0;
}

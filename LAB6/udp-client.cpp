#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int PORT = 8080;
const int MAX_BUFFER_SIZE = 1024;

int main() {
    // создание сокета
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // заполняем данные об адресе сервера
    struct sockaddr_in server_address;
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    char buffer[MAX_BUFFER_SIZE];
    std::cout << "Enter message: ";
    std::cin.getline(buffer, MAX_BUFFER_SIZE); // считываем некоторое сообщение от клиента

    while (true){
        // отправляем это сообщение
        int sent_bytes = sendto(client_socket, buffer, std::strlen(buffer), 0, (struct sockaddr*)&server_address, sizeof(server_address));
        if (sent_bytes < 0) {
            std::cerr << "Failed to send data\n";
            return 1;
        }

        // получаем новые данные от сервера
        int server_address_length = sizeof(server_address);
        std::memset(buffer, 0, MAX_BUFFER_SIZE);
        int received_bytes = recvfrom(client_socket, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&server_address, (socklen_t*)&server_address_length);
        if (received_bytes < 0) {
            std::cerr << "Failed to receive data\n";
            return 1;
        }

        // выводим их на экран
        std::cout << "Received " << received_bytes << " bytes from " << inet_ntoa(server_address.sin_addr) << ":" << ntohs(server_address.sin_port) << "\n";
        std::cout << "Data: " << buffer << "\n";
        sleep(1);
    }   

    close(client_socket);
    return 0;
}

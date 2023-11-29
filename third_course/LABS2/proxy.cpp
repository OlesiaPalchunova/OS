#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream> 
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <regex>

#define MAX_BUFFER_SIZE 4096
#define PORT 8080


std::unordered_map<std::string, std::string> cache;

std::string extractTargetUrl(const std::string& request) {
    std::istringstream iss(request);
    std::string firstLine;
    std::getline(iss, firstLine);

    size_t start = firstLine.find(' ') + 1;
    size_t end = firstLine.rfind(' ');
    return firstLine.substr(start, end - start);
}

std::string getDataFromCache(const std::string& url) {
    if (cache.find(url) != cache.end()) {
        return cache[url];
    }
    return "";
}

void saveDataToCache(const std::string& url, const std::string& data) {
    cache[url] = data;
}

std::string extractDomainFromUrl(const std::string& url) {
    size_t pos = url.find("://");
    if (pos != std::string::npos) {
        std::string remaining = url.substr(pos + 3);
        size_t slashPos = remaining.find('/');
        if (slashPos != std::string::npos) {
            return remaining.substr(0, slashPos);
        } else {
            return remaining;
        }
    }
    return url;
}

void* handleClientRequest(void* args) {
    int clientSocket = *((int*)args);
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
    std::string request(buffer, bytesRead);
    std::string targetUrl = extractTargetUrl(request);

    std::string cachedData = getDataFromCache(targetUrl);
    if (!cachedData.empty()) {
        write(clientSocket, cachedData.c_str(), cachedData.length());
        close(clientSocket);
        return nullptr;
    }

    struct addrinfo hints{};
    struct addrinfo* result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    std::cout << "Domain: " << extractDomainFromUrl(targetUrl.c_str()) << std::endl;
    if (getaddrinfo(extractDomainFromUrl(targetUrl.c_str()).c_str(), "http", &hints, &result) != 0) {
        std::cerr << "Failed to resolve the hostname" << std::endl;
        close(clientSocket);
        return nullptr;
    }

    int targetSocket = socket(AF_INET, SOCK_STREAM, 0);
    connect(targetSocket, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);

    write(targetSocket, request.c_str(), request.length());

    memset(buffer, 0, sizeof(buffer));
    std::string receivedData;
    while ((bytesRead = read(targetSocket, buffer, sizeof(buffer))) > 0) {
        receivedData.append(buffer, bytesRead);
        write(clientSocket, buffer, bytesRead);
        memset(buffer, 0, sizeof(buffer));
    }

    if (!receivedData.empty()) {
        saveDataToCache(targetUrl, receivedData);
    }

    close(targetSocket);
    close(clientSocket);
    return nullptr;
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create server socket." << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddr{};
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Failed to bind server socket." << std::endl;
        return 1;
    }

    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Failed to listen on server socket." << std::endl;
        return 1;
    }

    std::cout << "Now serving at :" << PORT << std::endl;

    while (true) {
        struct sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int* clientSocket = new int(accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen));
        if (*clientSocket == -1) {
            std::cerr << "Failed to accept client connection." << std::endl;
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, nullptr, handleClientRequest, (void*)clientSocket);
        pthread_detach(tid);
    }

    close(serverSocket);
    return 0;
}

//curl --proxy "http://localhost:8080" -v --insecure http://example.com/
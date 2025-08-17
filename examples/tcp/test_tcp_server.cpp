#include "src/tcp_communicator.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>

static const int PORT = 7473;  // Port for testing
static void die(const char* msg){ perror(msg); exit(1); }

int main() {
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) die("socket");
    
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (sockaddr*)&sa, sizeof(sa)) < 0) die("bind");
    if (listen(sockfd, 3) < 0) die("listen");
    
    std::cout << "TCP Server listening on port " << PORT << " ...\n";
    
    int addrlen = sizeof(sa);
    int cfd = accept(sockfd, (struct sockaddr *)&sa, (socklen_t*)&addrlen);
    if (cfd < 0) die("accept");
    std::cout << "TCP Server accepted connection from " << inet_ntoa(sa.sin_addr) << " on port " << ntohs(sa.sin_port) << " ...\n";
    
    // Create TCP communicator
    TCPCommunicator tcp_comm(cfd);
    
    // Receive message
    char buffer[256] = {0};
    std::cout << "Server waiting to receive message...\n";
    if (tcp_comm.recv(buffer, sizeof(buffer)) > 0) {
        std::cout << "Server received: " << buffer << std::endl;
    } else {
        std::cout << "Server failed to receive message\n";
    }
    
    // Send response
    const char* response = "Hello from TCP server";
    std::cout << "Server sending response...\n";
    if (tcp_comm.send(response, strlen(response) + 1) == strlen(response) + 1) {
        std::cout << "Server sent response\n";
    } else {
        std::cout << "Server failed to send response\n";
    }
    std::cout << "Server closing connection...\n";
    
    close(cfd);
    close(sockfd);
    
    return 0;
}
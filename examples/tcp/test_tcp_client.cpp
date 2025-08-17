#include "src/tcp_communicator.h"
#include <sys/socket.h>
#include <netinet/in.h>
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
    
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Connect to localhost
    
    if (connect(sockfd, (sockaddr*)&sa, sizeof(sa)) < 0) die("connect");
    
    // Create TCP communicator
    TCPCommunicator tcp_comm(sockfd);
    
    // Send message
    const char* message = "Hello from TCP client";
    std::cout << "Client sending message...\n";
    if (tcp_comm.send(message, strlen(message) + 1) == strlen(message) + 1) {
        std::cout << "Client sent message\n";
    } else {
        std::cout << "Client failed to send message\n";
    }
    
    // Receive response
    char buffer[256] = {0};
    std::cout << "Client waiting to receive response...\n";
    if (tcp_comm.recv(buffer, sizeof(buffer)) > 0) {
        std::cout << "Client received: " << buffer << std::endl;
    } else {
        std::cout << "Client failed to receive response\n";
    }
    std::cout << "Client closing connection...\n";
    
    close(sockfd);
    
    return 0;
}
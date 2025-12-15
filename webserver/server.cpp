#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int PORT = 8080;
const int BACKLOG = 10;

int main() {
    std::cout << "Starting server...\n" << std::endl;

    // 1. Create a socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket\n" << std::endl;
        return 1;
    }
    std::cout << "Socket created\n" << std::endl;

    // 2. Set socket options (allows quick restart)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options\n";
        return 1;
    }

    // 3. Bind socket to address and port
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Faild to bind socket\n";
        close(server_fd);
        return 1;
    }
    std::cout << "Socket bound to port " << PORT << "\n";

    // 4. Listen for connections
    if (listen(server_fd, BACKLOG) < 0) {
        std::cerr << "Failed to listen\n";
        close(server_fd);
        return 1;
    }
    std::cout << "Server listening on http://localhost:" << PORT << "\n";
    std::cout << "Press Ctrl+C to stop \n\n";

    //5. Accept and hadle connections
    while (true) {
        std::cout << "Waiting for connection...\n";

        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection \n";
            continue;
        }

        std::cout << "Client connected:\n";

        // Read the HTTP request
        char buffer[4096] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
            std::cout << "Request received:\n";
            std::cout << "----\n" << buffer << "----\n\n";

            // Send HTTP response
            std::string response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html>\n"
                "<head><title>My C++ Server</title></head>\n"
                "<body>\n"
                "  <h1>Hello from C++ Server!</h1>\n"
                "  <p>This is a basic web server written in C++.</p>\n"
                "  <p>Server is running on port " + std::to_string(PORT) + "</p>\n"
                "</body>\n"
                "</html>";
            
            send(client_fd, response.c_str(), response.length(), 0);
            std::cout << "✓ Response sent\n\n";

        }
        close(client_fd);
    }
    close(server_fd);
    return 0;
}
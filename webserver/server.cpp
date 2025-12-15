#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>

const int PORT = 8080;
const int BACKLOG = 10;

// Thread-safe logging
std::mutex log_mutex;

void log(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << "[Thread " << std::this_thread::get_id() << "] " 
              << message << std::endl;
}

// Handle a single client connection
void handle_client(int client_fd, int connection_num) {
    log("Connection #" + std::to_string(connection_num) + " - Client connected");
    
    // Read HTTP request
    char buffer[4096] = {0};
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    
    if (bytes_read > 0) {
        std::string request(buffer);
        
        // Extract the first line (GET /path HTTP/1.1)
        size_t first_line_end = request.find("\r\n");
        std::string first_line = request.substr(0, first_line_end);
        log("Connection #" + std::to_string(connection_num) + " - " + first_line);
        
        // Simulate some processing time  
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Send response
        std::string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html>\n"
            "<head><title>Multi-threaded Server</title></head>\n"
            "<body>\n"
            "  <h1>Multi-threaded C++ Server!</h1>\n"
            "  <p>Connection #" + std::to_string(connection_num) + "</p>\n"
            "  <p>Handled by thread: " + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())) + "</p>\n"
            "  <p>Each connection runs in its own thread!</p>\n"
            "</body>\n"
            "</html>";
        
        send(client_fd, response.c_str(), response.length(), 0);
        log("Connection #" + std::to_string(connection_num) + " - Response sent");
    }
    
    close(client_fd);
    log("Connection #" + std::to_string(connection_num) + " - Connection closed");
}

int main() {
    std::cout << "Starting multi-threaded server...\n";
    
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind\n";
        return 1;
    }
    
    // Listen
    if (listen(server_fd, BACKLOG) < 0) {
        std::cerr << "Failed to listen\n";
        return 1;
    }
    
    std::cout << "✓ Server listening on http://localhost:" << PORT << "\n";
    std::cout << "✓ Multi-threading enabled\n";
    std::cout << "✓ Each connection gets its own thread\n\n";
    
    int connection_counter = 0;
    
    // Accept connections
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection\n";
            continue;
        }
        
        connection_counter++;
        
        // Spawn a new thread for each connection
        std::thread client_thread(handle_client, client_fd, connection_counter);
        client_thread.detach();  // Let thread run independently
    }
    
    close(server_fd);
    return 0;
}
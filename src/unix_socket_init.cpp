#include "unix_socket_server.hpp"
#include <iostream>

bool initSocket() {
  mathboard::UnixSocketServer server;
  bool running = true;

  if (!server.Init("socket")) {
    std::cerr << "Failed to initialize the Unix Socket Server" << std::endl;
    return 1;
  }

  server.Listen(server.getServerSocketFd());

  std::cout << "Server is listening" << std::endl;

  sockaddr_un client_addr;
  std::int32_t client_fd;

  // Loop to handle request
  while (running) {
    if (server.Accept(server.getServerSocketFd(), client_fd,
                      (sockaddr &)client_addr)) {
      std::cout << "Client connected!" << std::endl;

      // Message reading and response
      char buffer[256];
      ssize_t bytesRead = read(client_fd, buffer, sizeof(buffer) - 1);

      if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "Data received from the client : " << buffer << std::endl;

        if (server.WriteString(client_fd, "Data received on the cpp!")) {
          std::cout << "Response send!" << std::endl;
        } else {
          std::cerr << "Failed to send response to the client" << std::endl;
        }
      } else {
        std::cerr << "Failed to read message from the client" << std::endl;
      }

      // Close client connection
      close(client_fd);
    } else {
      std::cerr << "Failed to accept the connection" << std::endl;
    }
  }

  remove("socket");

  return true;
}

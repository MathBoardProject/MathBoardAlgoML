// header
#include "run_unix_socket.hpp"

// local
#include "../unix_socket_server/unix_socket_server.hpp"

// std
#include <iostream>

namespace mathboard {

bool runUnixSocket() {
  std::unique_ptr<UnixSocketServer> server(UnixSocketServer::Instance());

  bool running = true;

  if (!server->Init("socket.sock")) {
    std::cerr << "Failed to initialize the Unix Socket Server" << std::endl;
    return false;
  }

  server->Listen();

  std::cout << "Server is listening" << std::endl;

  void *client_addr;
  int client_fd;

  // Loop to handle request
  while (running) {
    if (server->Accept(client_fd, &client_addr)) {
      std::cout << "Client connected!" << std::endl;

      // Message reading and response
      std::vector<unsigned char> buffer(256);

      server->Read(client_fd, buffer);

      if (buffer.size() > 0) {
        std::string message = std::string(buffer.begin(), buffer.end());

        std::cout << "Data received from the client: " << message << std::endl;

        if (server->WriteString(client_fd, "Data received on the CPP!")) {
          std::cout << "Response send!" << std::endl;
        } else {
          std::cerr << "Failed to send response to the client" << std::endl;
        }
      } else {
        std::cerr << "Failed to read message from the client" << std::endl;
      }

    } else {
      std::cerr << "Failed to accept the connection" << std::endl;
    }
  }

  return true;
}

} // namespace mathboard

// header
#include "run_unix_socket.hpp"

// local
#include "../unix_socket_server/unix_socket_server.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>

namespace mathboard {

bool runUnixSocket() {
  std::unique_ptr<UnixSocketServer> server(UnixSocketServer::Instance());

  bool running = true;

  if (!server->Init("socket.sock")) {
    spdlog::error(
        "[runUnixSocket] - Failed to initialize the Unix Socket Server.\n");
    return false;
  }

  server->Listen();

  spdlog::info("[runUnixSocket] - Server is listening.\n");

  void *client_addr;
  int client_fd;

  // Loop to handle request
  while (running) {
    if (server->Accept(client_fd, &client_addr)) {
      spdlog::info("[runUnixSocket] - Client connected.\n");

      // Message reading and response
      std::vector<unsigned char> buffer(256);

      server->Read(client_fd, buffer);

      if (buffer.size() > 0) {
        std::string message = std::string(buffer.begin(), buffer.end());

        spdlog::info("[runUnixSocket] - Data received from the client: {}\n",
                     message);

        if (server->WriteString(client_fd, "Data received on the CPP!")) {
          spdlog::info("[runUnixSocket] - Response send.\n");

        } else {
          spdlog::error(
              "[runUnixSocket] - Failed to send response to the client.\n");
        }
      } else {
        spdlog::error(
            "[runUnixSocket] - Failed to read message from the client.\n");
      }

    } else {
      spdlog::error("[runUnixSocket] - Failed to accept the connection.\n");
    }
  }

  return true;
}

} // namespace mathboard

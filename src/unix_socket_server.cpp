// header
#include "unix_socket_server.hpp"

// std
#include <fstream>

namespace mathboard {

bool UnixSocketServer::Init(const std::filesystem::path &socket_path) {
  m_SocketPath = socket_path;

  // Create the server's socket
  m_SocketServFd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (m_SocketServFd < 0) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream << "[UnixSocketServer::Init] Error: Could not open the socket."
                 << std::endl;

    debug_stream.close();
    return false;
  }

  // If there was any, remove old socket file
  remove(m_SocketPath);

  // Init Unix socket address struct
  sockaddr_un serv_addr{};
  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, m_SocketPath.c_str());

  // Bind the address to the socket
  if (bind(m_SocketServFd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream
        << "[UnixSocketServer::Init] Error: Could not bind the address to "
           "the socket."
        << std::endl;

    debug_stream.close();
    return false;
  }

  return true;
}

void UnixSocketServer::Listen(const std::int32_t socket_fd) {
  // Sleep until an incoming connection appears
  // Queue up to 16 requests
  listen(socket_fd, 16);
}

bool UnixSocketServer::Accept(const std::int32_t socket_serv_fd,
                              std::int32_t &socket_cli_fd,
                              sockaddr &sock_cli_addr) {
  socklen_t sock_cli_len = sizeof(sock_cli_addr);

  // Accept the incoming connection
  socket_cli_fd = accept(socket_serv_fd, &sock_cli_addr, &sock_cli_len);

  if (socket_cli_fd < 0) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream
        << "[UnixSocketServer::Init] Error: Could not accept the connection"
        << std::endl;

    debug_stream.close();
    return false;
  }

  return true;
}

// Read bytes from the socket socket_fd and write it into
// buffer
bool UnixSocketServer::Read(const std::int32_t socket_fd,
                            std::vector<std::uint8_t> &buffer) {
  // Read the data into the buffer
  std::ptrdiff_t byte_read = read(socket_fd, buffer.data(), buffer.size() - 1);

  if (byte_read < 0) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream
        << "[UnixSocketServer::Init] Error: Could not read from the socket"
        << std::endl;

    debug_stream.close();
    return false;
  }

  return true;
}

bool UnixSocketServer::Write(const std::int32_t socket_fd,
                             const std::vector<std::uint8_t> &buffer) {
  // Write buffer to the socket
  std::ptrdiff_t byte_read = write(socket_fd, buffer.data(), buffer.size());

  if (byte_read < 0) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream
        << "[UnixSocketServer::Init] Error: Could not write to the socket"
        << std::endl;

    debug_stream.close();
    return false;
  }

  return true;
}

bool UnixSocketServer::WriteString(const std::int32_t socket_fd,
                                   const std::string &msg) {
  // Write message to the socket
  std::ptrdiff_t byte_read = write(socket_fd, msg.c_str(), msg.size());

  if (byte_read < 0) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream
        << "[UnixSocketServer::Init] Error: Could not write to the socket"
        << std::endl;

    debug_stream.close();
    return false;
  }

  return true;
}

} // namespace mathboard

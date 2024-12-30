#pragma once
#ifdef __linux__

// prototype
#include "unix_socket_server.hpp"

// std
#include <cstdint>
#include <filesystem>
#include <vector>

// linux std
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace mathboard {

class LinuxUnixSocketServer : public UnixSocketServer {
public:
  ~LinuxUnixSocketServer() {
    if (m_SocketServFd > 2) {
      close(m_SocketServFd);
      remove(m_SocketPath);
    }
  }

  bool Init(const std::filesystem::path &socket_path) override;

  void Listen() override;

  bool Accept(int &socket_cli_fd, void **sock_cli_addr) override;

  bool Read(const std::int32_t socket_fd,
            std::vector<unsigned char> &buffer) override;

  bool Write(const std::int32_t socket_fd,
             const std::vector<unsigned char> &buffer) override;

  bool WriteString(const std::int32_t socket_fd,
                   const std::string &msg) override;
};

} // namespace mathboard

#endif

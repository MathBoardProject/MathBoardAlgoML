#pragma once

// std
#include <cstdint>
#include <filesystem>
#include <vector>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#elif _WIN32
// SOME WINDOWS INCLUDES
#else
#error shucks!
#endif

namespace mathboard {

class UnixSocketServer {
public:
  ~UnixSocketServer() {
    if (m_SocketServFd > 2) {
      close(m_SocketServFd);
    }
  }

  bool Init(const std::filesystem::path &socket_path);

  static void Listen(const std::int32_t socket_fd);

  static bool Accept(const std::int32_t socket_serv_fd,
                     std::int32_t &socket_cli_fd, sockaddr &sock_cli_addr);

  static bool Read(const std::int32_t socket_fd,
                   std::vector<std::uint8_t> &buffer);

  static bool Write(const std::int32_t socket_fd,
                    const std::vector<std::uint8_t> &buffer);

  std::int32_t GetSocketFd() const { return m_SocketServFd; }

  static bool WriteString(const std::int32_t socket_fd, const std::string &msg);

  std::int32_t getServerSocketFd() const { return m_SocketServFd; }

private:
  std::int32_t m_SocketServFd = -1;
  std::filesystem::path m_SocketPath;
};

} // namespace mathboard

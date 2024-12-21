#pragma once

// std
#include <cstdint>
#include <filesystem>
#include <vector>

namespace mathboard {

class UnixSocketServer {
public:
  virtual ~UnixSocketServer() = default;

  static UnixSocketServer *Instance();

  virtual bool Init(const std::filesystem::path &socket_path) = 0;

  virtual void Listen() = 0;

  virtual bool Accept(int &socket_cli_fd, void **sock_cli_addr) = 0;

  virtual bool Read(const std::int32_t socket_fd,
                    std::vector<unsigned char> &buffer) = 0;

  virtual bool Write(const std::int32_t socket_fd,
                     const std::vector<unsigned char> &buffer) = 0;

  virtual bool WriteString(const std::int32_t socket_fd,
                           const std::string &msg) = 0;

  virtual std::int32_t GetServerSocketFd() const { return m_SocketServFd; }

protected:
  int m_SocketServFd = -1;
  std::filesystem::path m_SocketPath{};
};

} // namespace mathboard

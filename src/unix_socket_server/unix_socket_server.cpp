// header
#include "unix_socket_server.hpp"
#include "linux_unix_socket_server.hpp"

namespace mathboard {

UnixSocketServer *UnixSocketServer::Instance() {
#ifdef __linux__
  UnixSocketServer *instance = new LinuxUnixSocketServer();

#elif _WIN32
  // TODO

#else
#error shucks!

#endif

  return instance;
}

} // namespace mathboard

#include "run_unix_socket.hpp"
#include <iostream>

int main() {
  if (!mathboard::runUnixSocket()) {
    std::cerr << "Failed to init the unix socket" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Unix Socket initialised succesfuly" << std::endl;
  return EXIT_SUCCESS;
}

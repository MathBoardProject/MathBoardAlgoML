#include "unix_socket_init.hpp"
#include <iostream>

int main() {
  if (!initSocket()) {
    std::cerr << "Failed to init the unix socket" << std::endl;
    return 1;
  }

  std::cout << "Unix Socket initialised succesfuly" << std::endl;
  return 0;
}
// local
#include "helper/solve_equation.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    spdlog::error("[main]: Usage: {} <image_path>", argv[0]);
    return EXIT_FAILURE;
  }

  const std::string imagePath = argv[1];

  mathboard::solveImage(imagePath);

  return EXIT_SUCCESS;
}

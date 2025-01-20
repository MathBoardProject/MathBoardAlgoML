// local
#include "model.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>

// std
#include <iostream>

int main() {
  mathboard::Model model("models/converted_model.tflite");
  std::cout << "created\n";
  return EXIT_SUCCESS;
}

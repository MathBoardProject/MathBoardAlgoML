// header
#include "mnist_loader.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>

// std
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace mathboard {
namespace mnistLoader {

uint32_t readBigEndian(std::ifstream &stream) { // Reads 4 bytes
  uint32_t result = 0;
  for (int i = 0; i < 4; ++i) {                                  // bytes
    result = (result << 8) | static_cast<uint8_t>(stream.get()); // bits
  }
  return result;
}

MNISTData loadMNIST(const std::string &filename) {
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    spdlog::error("Failed to open file :" + filename);
  }

  uint32_t magicNumber = readBigEndian(file);
  MNISTData data;

  if (magicNumber == 0x00000803) { // Images
    std::cout << "Loading images from file: " << filename << std::endl;

    uint32_t numImages = readBigEndian(file);
    uint32_t numRows = readBigEndian(file);
    uint32_t numCols = readBigEndian(file);

    data.images.resize(numImages, std::vector<uint32_t>(numRows * numCols));

    for (uint32_t i = 0; i < numImages; i++) {
      file.read(reinterpret_cast<char *>(data.images[i].data()),
                numRows * numCols);
    }
  } else if (magicNumber == 0x00000801) { // Labels
    std::cout << "Loading labels from file: " << filename << std::endl;

    uint32_t numLabels = readBigEndian(file);
    data.labels.resize(numLabels);
    file.read(reinterpret_cast<char *>(data.labels.data()), numLabels);
  } else {
    spdlog::error("Unrecognized magic number: " + std::to_string(magicNumber));
  }

  return data;
}

} // namespace mnistLoader
} // namespace mathboard

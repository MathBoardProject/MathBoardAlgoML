#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace mathboard {
namespace mnistLoader {
uint32_t readBigEndian(std::fstream &stream);

struct MNISTData {
  std::vector<std::vector<uint32_t>> images;
  std::vector<uint32_t> labels;
};

MNISTData loadMNIST(const std::string &filename);
} // namespace mnistLoader
} // namespace mathboard

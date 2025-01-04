#ifndef MNIST_LOADER
#define MNIST_LOADER

#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mathboard {
uint32_t readBigEndian(std::fstream &stream);

struct MNISTData {
  std::vector<std::vector<uint32_t>> images;
  std::vector<uint32_t> labels;
};

MNISTData loadMNIST(const std::string &filename);

std::vector<uint8_t> flattenImage(const std::vector<uint8_t> &image);

std::vector<std::vector<uint8_t>> flattenImages(const std::vector<std::vector<uint8_t>> &images);
} // namespace mathboard

#endif
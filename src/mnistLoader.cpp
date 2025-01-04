#include "mnistLoader.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace mathboard {   // maybe change to mnistLoader
namespace mnistLoader { // Not sure if it is great idea, but looks fine.

uint32_t readBigEndian(ifstream &stream) { // Reads 4 bytes
  uint32_t result = 0;
  for (int i = 0; i < 4; ++i) {                                  // bytes
    result = (result << 8) | static_cast<uint8_t>(stream.get()); // bits
  }
  return result;
}

MNISTData loadMNIST(const string &filename) {
  ifstream file(filename, ios::binary);
  if (!file.is_open()) {
    throw runtime_error("Failed to open file: " + filename);
  }

  uint32_t magicNumber = readBigEndian(file);
  MNISTData data;

  if (magicNumber == 0x00000803) { // Images
    cout << "Loading images from file: " << filename << endl;

    uint32_t numImages = readBigEndian(file);
    uint32_t numRows = readBigEndian(file);
    uint32_t numCols = readBigEndian(file);

    data.images.resize(numImages, vector<uint32_t>(numRows * numCols));

    for (uint32_t i = 0; i < numImages; i++) {
      file.read(reinterpret_cast<char *>(data.images[i].data()),
                numRows * numCols);
    }
  } else if (magicNumber == 0x00000801) { // Labels
    cout << "Loading labels from file: " << filename << endl;

    uint32_t numLabels = readBigEndian(file);
    data.labels.resize(numLabels);
    file.read(reinterpret_cast<char *>(data.labels.data()), numLabels);
  } else {
    throw runtime_error("Unrecognized magic number: " + to_string(magicNumber));
  }

  return data;
}

vector<uint8_t>
flattenImage(const vector<uint8_t> &image) { // Converts to 1Dimension-Arrays
  vector<uint8_t> flattened(image.begin(), image.end());
  return flattened;
}

vector<vector<uint8_t>> flattenImages(const vector<vector<uint8_t>> &images) {
  vector<vector<uint8_t>> flattenedImages;
  for (const auto &image : images) {
    flattenedImages.push_back(flattenImage(image));
  }
  return flattenedImages;
}
} // namespace mnistLoader
} // namespace mathboard
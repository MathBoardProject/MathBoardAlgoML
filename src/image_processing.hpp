#pragma once

// local
#include "grid.hpp"
#include "stroke.hpp"

// opencv
#include <opencv2/core/mat.hpp>

// std
#include <filesystem>

namespace mathboard {
// take path to svg file and transform it into pixel representation using
// cv::Mat
cv::Mat RasterizeImage(const std::filesystem::path &filename);

// `input_mat` has to be grayscale
// crop image to tightly fit symbol on it
cv::Mat CropToSymbol(const cv::Mat &input_mat);

// Return grascaled version of input image
cv::Mat GrayScaleImage(const cv::Mat &input_mat);

// Return image binarized using threshold.
cv::Mat BinarizeImage(const cv::Mat &input_mat);

// Returns image string
std::string RecognizeText(const cv::Mat &img);

// Combine all given strokes into a single matrix.
cv::Mat CombineStrokes(const std::vector<mathboard::Stroke *> &strokes);

cv::Mat ResizeToMNISTFormat(const cv::Mat &input_mat);

// Generate all possible combinations of elements from the container,
// regardless of order.
// Example: input = [1, 2] -> output = [[1], [2], [1, 2]]
template <typename T>
std::vector<std::vector<T>>
GenerateCombinations(const std::vector<T> &elements) {
  // Total combinations = 2^n
  const std::size_t total = 1 << elements.size();

  std::vector<std::vector<T>> all_combinations;
  all_combinations.resize(total - 1);
  // start with one to avoid empty combination
  for (std::size_t i = 1; i < total; ++i) {
    for (std::size_t j = 0; j < elements.size(); ++j) {
      // Check if the j-th bit is set
      if (i & (1 << j)) {
        all_combinations[i - 1].push_back(elements[j]);
      }
    }
  }
  return all_combinations;
}

// returns instance of Grid class with all images put on
// their positions ready to further interpreatation
// it sets grid cell size and boundaries of it
// automaticaly
Grid<mathboard::Stroke> inline PlaceOnGrid(
    std::vector<mathboard::Stroke> &strokes) {
  // calculate boundaries of grid
  cv::Point2f bot_right_corner{0, 0};
  cv::Point2f top_left_corner{INFINITY, INFINITY};
  for (std::size_t i = 0; i < strokes.size(); i++) {
    const cv::Point2i stroke_pos = strokes[i].GetPosition();
    const cv::Size2i stroke_size = strokes[i].GetSize();

    if (bot_right_corner.x < stroke_pos.x + stroke_size.width) {
      bot_right_corner.x = stroke_pos.x + stroke_size.width;
    }
    if (top_left_corner.x > stroke_pos.x) {
      top_left_corner.x = stroke_pos.x;
    }
    if (bot_right_corner.y < stroke_pos.y + stroke_size.height) {
      bot_right_corner.y = stroke_pos.y + stroke_size.height;
    }
    if (top_left_corner.y > stroke_pos.y) {
      top_left_corner.y = stroke_pos.y;
    }
  }

  // calculate average size of stroke
  cv::Size2i average_stroke_size = cv::Size2i(0, 0);
  for (const auto &stroke : strokes) {
    average_stroke_size.width += stroke.GetSize().width;
    average_stroke_size.height += stroke.GetSize().width;
  }
  average_stroke_size.width /= static_cast<float>(strokes.size());
  average_stroke_size.height /= static_cast<float>(strokes.size());

  // create grid
  Grid<mathboard::Stroke> grid(top_left_corner, bot_right_corner,
                               average_stroke_size);
  for (std::size_t i = 0; i < strokes.size(); i++) {
    grid.Insert(&strokes[i]);
  }
  return grid;
}
} // namespace mathboard
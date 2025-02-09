#pragma once

#include "stroke.hpp"
#include <cstddef>
#include <cstdio>
#include <spdlog/spdlog.h>
#include <vector>
namespace mathboard {
struct Prediction {
  Prediction() = default;
  Prediction(double confidance, int number, std::size_t index,
             std::vector<mathboard::Stroke> strokes = {})
      : index(index), confidance(confidance), number(number), strokes(strokes) {
    if (confidance < 0.0 || confidance > 1.0) {
      spdlog::error(
          "[Prediction::Prediction(): confidance isn't in range(0.0 - 1.0)]");
    }
  }
  // prediction's index
  std::size_t index{0};
  double confidance{0.0};
  // predicted number
  int number{0};
  // strokes creating number
  std::vector<mathboard::Stroke> strokes;
};
} // namespace mathboard
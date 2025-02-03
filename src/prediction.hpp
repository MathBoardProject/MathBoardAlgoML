#pragma once

#include <cstddef>
namespace mathboard {
struct Prediction {
  Prediction() = default;
  Prediction(double confidance, int number, std::size_t symbol_index)
      : confidance(confidance), guessed_number(number),
        symbol_index(symbol_index) {}
  double confidance{0.0};
  int guessed_number{0};
  std::size_t symbol_index{0};
};
} // namespace mathboard
#pragma once

// local
#include "image_processing.hpp"
#include "stroke.hpp"

// libs
// spdlog
#include <algorithm>
#include <array>
#include <cstddef>
#include <opencv2/core/types.hpp>
#include <spdlog/spdlog.h>

namespace mathboard {

class Symbol {
public:
  Symbol() = default;
  Symbol(std::size_t index, char label, double confidence,
         const std::vector<mathboard::Stroke> &strokes)
      : m_Index(index), m_Confidence(confidence), m_Label(label),
        strokes(strokes) {
    if (m_Confidence < 0.0 || m_Confidence > 1.0) {
      spdlog::error("[Symbol::Symbol()]: confidence is out of range(0 - 1)");
    }
    if (IsValid(m_Label) == false) {
      spdlog::error("[Symbol::Symbol()]: label is unknown");
    }
    if (strokes.empty() == true) {
      spdlog::error("[Symbol::Symbol()]: strokes is empty");
    }
    m_Matrix = CombineStrokes(strokes);
    for (const auto &stroke : strokes) {
      m_Rect = m_Rect | stroke.GetRect();
    }
  }
  Symbol(std::size_t index, char label, double confidence)
      : m_Index(index), m_Confidence(confidence), m_Label(label) {
    if (m_Confidence < 0.0 || m_Confidence > 1.0) {
      spdlog::error("[Symbol::Symbol()]: confidence is out of range(0 - 1)");
    }
    if (IsValid(m_Label) == false) {
      spdlog::error("[Symbol::Symbol()]: label argument is unknown");
    }
  }

public:
  std::size_t GetIndex() const { return m_Index; }

  double GetConfidence() const { return m_Confidence; }

  char GetLabel() const { return m_Label; }

  cv::Rect GetRect() const { return m_Rect; }

  cv::Mat GetMatrix() const { return m_Matrix; }

  std::vector<mathboard::Stroke> GetStrokes() const { return strokes; }

private:
  // TODO:
  // I can make it template and standarize it
  static bool IsValid(char label) {
    if (std::find(m_ValidLabels.begin(), m_ValidLabels.end(), label) ==
        m_ValidLabels.end()) {
      return false;
    }
    return true;
  };

private:
  cv::Mat m_Matrix;
  cv::Rect m_Rect;
  std::size_t m_Index{0};
  // model's confidence score for recognizing this symbol
  double m_Confidence{0.0};
  // what the symbol is representing
  char m_Label{' '};
  // strokes creating number
  std::vector<mathboard::Stroke> strokes;
  constexpr static std::array<char, 10> m_ValidLabels = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
};
} // namespace mathboard
#pragma once

// libs
// OpenCV
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

namespace mathboard {

// class holding basic information about image of stroke
class Stroke {
public:
  Stroke() = default;
  Stroke(std::size_t index, float pos_x, float pos_y, cv::Mat grayscale_image,
         double scale_factor = 1.0 / 255.0);
  Stroke(std::size_t index, cv::Point2f position, cv::Mat grayscale_image,
         double scale_factor = 1.0 / 255.0);
  bool operator==(const Stroke &other) const {
    return this->m_Index == other.m_Index;
  }

public:
  cv::Point GetPosition() const { return m_Position; }
  std::size_t GetIndex() const { return m_Index; }
  cv::Mat GetMatrix() const { return m_Matrix; }
  cv::Size2i GetSize() const { return m_Matrix.size(); }
  cv::Rect GetRect() const {
    return {m_Position, GetSize()};
  }

private:
  std::size_t m_Index{0};
  cv::Point2f m_Position{0.0f, 0.0f};
  cv::Mat m_Matrix;
};
} // namespace mathboard
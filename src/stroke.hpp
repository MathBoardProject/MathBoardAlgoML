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
  Stroke(int index, int pos_x, int pos_y, cv::Mat grayscale_image,
         double scale_factor = 1.0 / 255.0);
  Stroke(int index, cv::Point2i position, cv::Mat grayscale_image,
         double scale_factor = 1.0 / 255.0);
  bool operator==(const Stroke &other) {
    return this->m_Index == other.m_Index;
  }

public:
  cv::Point2i GetPosition() const { return m_Position; }
  std::uint32_t GetIndex() const { return m_Index; }
  cv::Mat GetMatrix() const { return m_Matrix; }
  cv::Size2i GetSize() const { return m_Matrix.size(); }
  cv::Rect2i GetBoundingBox() const {
    return cv::Rect2i(GetPosition(), GetSize());
  }

private:
  std::uint32_t m_Index{0};
  cv::Point2i m_Position{0, 0};
  cv::Mat m_Matrix;
};
} // namespace mathboard
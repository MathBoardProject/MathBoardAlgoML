#pragma once

// libs
// OpenCV
#include <opencv2/core/types.hpp>
namespace mathboard {

// class holding basic information about image of stroke
class Stroke {
public:
  Stroke() = default;
  Stroke(int index, float pos_x, float pos_y, cv::Mat grayscale_image);
  Stroke(int index, cv::Point2f position, cv::Mat grayscale_image);

public:
  cv::Point2f GetPosition() const { return m_Position; }
  cv::Rect GetBoundingBox() const { return m_BoundingBox; }
  int GetWidth() const { return m_BoundingBox.height; }
  int GetHeight() const { return m_BoundingBox.width; }
  std::vector<std::vector<cv::Point>> GetContours() const { return m_Contours; }
  std::uint32_t GetIndex() const { return m_Index; }
private:
  std::uint32_t m_Index{0};
  cv::Point2f m_Position;
  std::vector<std::vector<cv::Point>> m_Contours;
  cv::Rect m_BoundingBox;
};
} // namespace mathboard
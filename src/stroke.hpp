#pragma once

// lib
// OpenCV
#include <opencv2/opencv.hpp>
// spdlog
#include <spdlog/spdlog.h>

// std
#include <fstream>

namespace mathboard {

// class holding basic information about image of stroke
// `image` has to be grayscale
class Stroke {
public:
  Stroke() = default;
  Stroke(int index, cv::Point2f position, cv::Mat image)
      : m_Index(index), m_Position(position) {
    if (image.channels() != 1) {
      spdlog::error("[Stroke::Stroke]: Image isn't grayscale.\n");
    }

    cv::findContours(image, m_Contours, cv::RETR_CCOMP,
                     cv::CHAIN_APPROX_SIMPLE);
    for (std::size_t i = 0; i < m_Contours.size(); i++) {
      m_BoundingBox = m_BoundingBox | cv::boundingRect(m_Contours[i]);
    }
  }
  cv::Point2f GetPosition() const { return m_Position; }
  cv::Rect GetBoundingBox() const { return m_BoundingBox; }
  std::uint32_t GetIndex() const { return m_Index; }

private:
  std::uint32_t m_Index{0};
  cv::Point2f m_Position;
  std::vector<std::vector<cv::Point>> m_Contours;
  cv::Rect m_BoundingBox;
};
} // namespace mathboard
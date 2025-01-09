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
class Stroke {
public:
  Stroke() = default;
  Stroke(int index, float pos_x, float pos_y, cv::Mat grayscale_image)
      : m_Index(index), m_Position(cv::Point2f{pos_x, pos_y}) {
    if (grayscale_image.channels() != 1) {
      spdlog::error("[Stroke::Stroke]: grayscale_image isn't grayscale.\n");
    }

    cv::findContours(grayscale_image, m_Contours, cv::RETR_CCOMP,
                     cv::CHAIN_APPROX_SIMPLE);
    for (std::size_t i = 0; i < m_Contours.size(); i++) {
      m_BoundingBox = m_BoundingBox | cv::boundingRect(m_Contours[i]);
    }
  }
  Stroke(int index, cv::Point2f position, cv::Mat grayscale_image)
      : m_Index(index), m_Position(position) {
    if (grayscale_image.channels() != 1) {
      spdlog::error("[Stroke::Stroke]: grayscale_image isn't grayscale.\n");
    }

    cv::findContours(grayscale_image, m_Contours, cv::RETR_CCOMP,
                     cv::CHAIN_APPROX_SIMPLE);
    for (std::size_t i = 0; i < m_Contours.size(); i++) {
      m_BoundingBox = m_BoundingBox | cv::boundingRect(m_Contours[i]);
    }
  }
  cv::Point2f GetPosition() const { return m_Position; }
  cv::Rect GetBoundingBox() const { return m_BoundingBox; }
  std::vector<std::vector<cv::Point>> GetContours() const { return m_Contours; }
  std::uint32_t GetIndex() const { return m_Index; }

private:
  std::uint32_t m_Index{0};
  cv::Point2f m_Position;
  std::vector<std::vector<cv::Point>> m_Contours;
  cv::Rect m_BoundingBox;
};
} // namespace mathboard
// header
#include "stroke.hpp"

// lib
// spdlog
#include <spdlog/spdlog.h>

// std
#include <fstream>

namespace mathboard {
Stroke::Stroke(int index, float pos_x, float pos_y, cv::Mat grayscale_image)
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
Stroke::Stroke(int index, cv::Point2f position, cv::Mat grayscale_image)
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

} // namespace mathboard

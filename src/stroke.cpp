// header
#include "stroke.hpp"

// lib
// spdlog
#include <spdlog/spdlog.h>

namespace mathboard {
Stroke::Stroke(std::size_t index, float pos_x, float pos_y, cv::Mat grayscale_image,
               double scale_factor)
    : m_Index(index), m_Position(pos_x, pos_y) {
  if (grayscale_image.channels() != 1) {
    spdlog::error("[Stroke::Stroke]: grayscale_image isn't grayscale.\n");
  }
  grayscale_image.convertTo(m_Matrix, CV_32F, scale_factor);
}
Stroke::Stroke(std::size_t index, cv::Point2f position, cv::Mat grayscale_image,
               double scale_factor)
    : m_Index(index), m_Position(position) {
  if (grayscale_image.channels() != 1) {
    spdlog::error("[Stroke::Stroke]: grayscale_image isn't grayscale.\n");
  }
  grayscale_image.convertTo(m_Matrix, CV_32F, scale_factor);
}

} // namespace mathboard

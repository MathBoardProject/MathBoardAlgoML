#pragma once

#include <fstream>
#include <opencv2/opencv.hpp>
namespace mathboard {

class Stroke {
public:
  // class holding basic information about stroke
  // `image` has to be grayscale
  Stroke(int index, cv::Point2f position, cv::Mat image)
      : m_Index(index), m_Position(position) {
    if (image.channels() != 1) {
      std::ofstream debug_stream("debug_output.txt",
                                 std::ios::app); // Debug output stream
      debug_stream << "[Stroke::Stroke()] Error: image isn't grayscale\n";

      debug_stream.close();
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
  std::uint32_t m_Index;
  cv::Point2f m_Position;
  std::vector<std::vector<cv::Point>> m_Contours;
  cv::Rect m_BoundingBox;
};
} // namespace mathboard
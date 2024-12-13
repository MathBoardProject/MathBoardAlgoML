// header
#include "image_processing.hpp"

// std
#include <fstream>

namespace mathboard {
bool RasterizeImage(const std::filesystem::path &filename,
                    cv::Mat &output_mat) {
  if (filename.extension() != ".svg") {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream << "[RasterizeFile] Error: File extension is "
                 << filename.extension() << " instead of \".svg\"" << std::endl;

    debug_stream.close();

    return false;
  }
  cv::VideoCapture video_capture{};
  video_capture.open(filename.string());

  if (!video_capture.isOpened()) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream << "[RasterizeFile] Error: Could not open video. " << filename
                 << std::endl;

    debug_stream.close();

    return false;
  }

  if (!video_capture.read(output_mat)) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream << "[RasterizeFile] Error: video_capture is empty"
                 << std::endl;

    debug_stream.close();

    return false;
  }
  return true;
}

void CropImageToSymbol(const cv::Mat &input_mat, cv::Mat &output_mat) {
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(input_mat, contours, cv::RETR_CCOMP,
                   cv::CHAIN_APPROX_SIMPLE);
  cv::Rect bounding_box;
  for (std::size_t i = 0; i < contours.size(); i++) {
    bounding_box = bounding_box | cv::boundingRect(contours[i]);
  }
  output_mat = input_mat(bounding_box);
}
} // namespace mathboard
// local
#include "image_processing.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>

// std
#include <fstream>

namespace mathboard {
cv::Mat RasterizeImage(const std::filesystem::path &filename) {
  cv::Mat rasterized_image;
  if (filename.extension() != ".svg") {
    spdlog::error("[RasterizeFile]: File extension is {} instead of .svg\n",
                  filename.extension().string());
  }
  cv::VideoCapture video_capture{};
  video_capture.open(filename.string());

  if (!video_capture.isOpened()) {
    spdlog::error("[RasterizeFile]: Could not open the video {}\n",
                  filename.string());
  }

  if (!video_capture.read(rasterized_image)) {
    spdlog::error("[RasterizeFile]: Video capture is empty\n");
  }

  return rasterized_image;
}

cv::Mat CropImageToSymbol(const cv::Mat &input_mat) {
  std::vector<std::vector<cv::Point>> contours;
  // merges bounding boxes of all objects on image
  // into one bigger bounding box containing all
  // content of image
  cv::findContours(input_mat, contours, cv::RETR_CCOMP,
                   cv::CHAIN_APPROX_SIMPLE);
  cv::Rect bounding_box;
  for (std::size_t i = 0; i < contours.size(); i++) {
    bounding_box = bounding_box | cv::boundingRect(contours[i]);
  }
  return input_mat(bounding_box);
}

} // namespace mathboard
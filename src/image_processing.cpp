// local
#include "image_processing.hpp"
#include "grid.hpp"
#include "stroke.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>

// std
#include <fstream>

namespace mathboard {
bool RasterizeImage(const std::filesystem::path &filename,
                    cv::Mat &output_mat) {
  if (filename.extension() != ".svg") {
    spdlog::error("[RasterizeFile]: File extension is {} instead of .svg\n",
                  filename.extension().string());
    return false;
  }
  cv::VideoCapture video_capture{};
  video_capture.open(filename.string());

  if (!video_capture.isOpened()) {
    spdlog::error("[RasterizeFile]: Could not open the video {}\n",
                  filename.string());
    return false;
  }

  if (!video_capture.read(output_mat)) {
    spdlog::error("[RasterizeFile]: Video capture is empty\n");
    return false;
  }

  return true;
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

Grid PlceOnGrid(const std::vector<cv::Mat> &grayscale_images,
                const std::vector<cv::Point2f> &images_positions) {

  // create vector of strokes
  std::vector<mathboard::Stroke> strokes;
  strokes.resize(grayscale_images.size());
  for (std::size_t i = 0; i < strokes.size(); i++) {
    strokes[i] = Stroke(i, images_positions[i], grayscale_images[i]);
  }

  // calculate boundaries of grid
  cv::Point2f bot_right_corner{0, 0};
  cv::Point2f top_left_corner{INFINITY, INFINITY};
  for (std::size_t i = 0; i < strokes.size(); i++) {
    const cv::Point2f line_pos = strokes[i].GetPosition();
    const cv::Rect line_BB = strokes[i].GetBoundingBox();

    if (bot_right_corner.x < line_pos.x + line_BB.width) {
      bot_right_corner.x = line_pos.x + line_BB.width;
    }
    if (top_left_corner.x > line_pos.x) {
      top_left_corner.x = line_pos.x;
    }
    if (bot_right_corner.y < line_pos.y + line_BB.height) {
      bot_right_corner.y = line_pos.y + line_BB.height;
    }
    if (top_left_corner.y > line_pos.y) {
      top_left_corner.y = line_pos.y;
    }
  }

  // calculate avrage size of one image
  cv::Size2f avrage_size_of_image{0.0f, 0.0f};
  for (std::size_t i = 0; i < strokes.size(); i++) {
    avrage_size_of_image.width += strokes[i].GetBoundingBox().size().width;
    avrage_size_of_image.height += strokes[i].GetBoundingBox().size().height;
  }
  avrage_size_of_image.width /= static_cast<float>(strokes.size());
  avrage_size_of_image.height /= static_cast<float>(strokes.size());

  // create grid
  Grid grid(top_left_corner, bot_right_corner, avrage_size_of_image);
  grid.InsertObjects(strokes);
  return grid;
}

} // namespace mathboard
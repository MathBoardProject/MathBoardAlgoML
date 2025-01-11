// local
#include "image_processing.hpp"
#include "grid.hpp"
#include "stroke.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>
// tesseract
#include <tesseract/baseapi.h>

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

Grid PlaceOnGrid(std::vector<mathboard::Stroke> &strokes) {
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
  for (std::size_t i = 0; i < strokes.size(); i++) {
    grid.InsertStroke(&strokes[i]);
  }
  return grid;
}

cv::Mat GrayScaleImage(const cv::Mat &input_mat) {
  cv::Mat greyImg;
  cv::cvtColor(input_mat, greyImg, cv::COLOR_BGR2GRAY);
  return greyImg;
}

cv::Mat BinarizeImage(const cv::Mat &input_mat) {
  cv::Mat binarizedImg;
  cv::threshold(input_mat, binarizedImg, 128, 255, cv::THRESH_BINARY);
  return binarizedImg;
}

std::string RecognizeText(const cv::Mat &img) {
  tesseract::TessBaseAPI ocr;
  if (ocr.Init(nullptr, "eng", tesseract::OEM_LSTM_ONLY)) {
    spdlog::error("[RecognizeText]: Could not initialize Tesseract.\n");
    throw std::runtime_error(
        "[RecognizeText] Error: Could not initialize Tesseract.");
  }

  ocr.SetImage(img.data, img.cols, img.rows, 1, img.step);
  std::string text = ocr.GetUTF8Text();
  ocr.End();

  if (text.empty()) {
    spdlog::error("[RecognizeText]: OCR did not recognize any text.\n");
    throw std::runtime_error(
        "[RecognizeText] Error: OCR did not recognize any text.");
  }

  return text;
}

} // namespace mathboard

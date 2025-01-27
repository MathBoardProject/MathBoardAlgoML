// local
#include "image_processing.hpp"

// libs
// opencv
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
// spdlog
#include <spdlog/spdlog.h>
// tesseract
#include <tesseract/baseapi.h>

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

// local
#include "image_processing.hpp"

// libs
// spdlog
#include <opencv2/core/hal/interface.h>
#include <opencv2/imgproc.hpp>
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

cv::Mat CropToSymbol(const cv::Mat &input_mat) {
  if (input_mat.channels() != 1) {
    spdlog::error("[CropToSymbol]: input_mat isn't grayscale");
  }
  cv::Mat cropped_mat = input_mat;
  if (input_mat.type() != CV_8UC1) {
    double min_val = 0.0f;
    double max_val = 0.0f;
    cv::minMaxLoc(input_mat, &min_val, &max_val);
    input_mat.convertTo(cropped_mat, CV_8UC1, (max_val - min_val),
                        -min_val * 255.0 / (max_val - min_val));
  }
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(cropped_mat, contours, cv::RETR_CCOMP,
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

cv::Mat CombineStrokes(const std::vector<mathboard::Stroke *> &strokes) {
  cv::Point2i top_left_corner = cv::Point2i(INT32_MAX, INT32_MAX);
  cv::Point2i bot_right_corner = cv::Point2i(INT32_MIN, INT32_MIN);
  for (const auto &stroke : strokes) {
    const cv::Point2i stroke_top_left_corner = stroke->GetPosition();
    const cv::Point2i stroke_bot_right_corner =
        cv::Point2i(stroke->GetPosition().x + stroke->GetSize().width,
                    stroke->GetPosition().y + stroke->GetSize().height);

    if (stroke_top_left_corner.x < top_left_corner.x) {
      top_left_corner.x = stroke_top_left_corner.x;
    }
    if (stroke_top_left_corner.y < top_left_corner.y) {
      top_left_corner.y = stroke_top_left_corner.y;
    }
    if (stroke_bot_right_corner.x > bot_right_corner.x) {
      bot_right_corner.x = stroke_bot_right_corner.x;
    }
    if (stroke_bot_right_corner.y > bot_right_corner.y) {
      bot_right_corner.y = stroke_bot_right_corner.y;
    }
  }
  cv::Mat symbol =
      cv::Mat::zeros(bot_right_corner.y - top_left_corner.y,
                     bot_right_corner.x - top_left_corner.x, CV_32F);
  for (auto &stroke : strokes) {
    cv::Point offset = stroke->GetPosition() - top_left_corner;
    cv::Rect roi(offset, stroke->GetSize());
    cv::Mat symbol_roi = symbol(roi);
    cv::max(symbol_roi, stroke->GetMatrix(), symbol_roi);
  }
  return symbol;
}
} // namespace mathboard

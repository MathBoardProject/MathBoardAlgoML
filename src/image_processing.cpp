// header
#include "image_processing.hpp"

// libs
// opencv
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
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

cv::Mat CropToSymbol(const cv::Mat &input_mat) {
  if (input_mat.channels() != 1) {
    spdlog::error("[CropToSymbol]: input_mat isn't grayscale");
  }
  cv::Mat cropped_mat;
  input_mat.copyTo(cropped_mat);
  if (input_mat.type() != CV_8UC1) {
    cropped_mat *= 255.0;
    double min_val = 0.0f;
    double max_val = 0.0f;
    cv::minMaxLoc(cropped_mat, &min_val, &max_val);
    cropped_mat.convertTo(cropped_mat, CV_8UC1, (max_val - min_val),
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

// it won't work unless size of strokes overlap with pixels on img passed 
cv::Mat CombineStrokes(const std::vector<mathboard::Stroke> &strokes) {
  if (strokes.empty()) {
    spdlog::error("[CombineStrokes()]: strokes vector is empty");
  }
  // Compute bounding box
  cv::Rect combined_rect;
  for (const auto &stroke : strokes) {
      combined_rect |= stroke.GetRect();
  }

  // Create empty matrix
  cv::Mat stroke_combination = cv::Mat::zeros(combined_rect.size(), CV_32F);
  for (const auto &stroke : strokes) {
    if (stroke.GetMatrix().empty()) {
        continue;
    }
    cv::Point offset = stroke.GetPosition() - combined_rect.tl();
    cv::Rect roi(offset, stroke.GetSize());
  
    // Ensure the region of interest is within bounds
    if (roi.x >= 0 && roi.y >= 0 &&
        roi.x + roi.width <= stroke_combination.cols &&
        roi.y + roi.height <= stroke_combination.rows) {
        cv::Mat stroke_combination_roi = stroke_combination(roi);
        cv::max(stroke_combination_roi, stroke.GetMatrix(), stroke_combination_roi);
    }
  }
  return stroke_combination;
}

cv::Mat ResizeToMNISTFormat(const cv::Mat &input_mat) {
  cv::Mat output_mat;
  cv::resize(input_mat, output_mat, cv::Size2i(28, 28), 0, 0, cv::INTER_CUBIC);
  return output_mat;
}

std::vector<mathboard::Stroke>
FindIntersectingStrokes(const mathboard::Stroke &target_stroke,
                        const std::vector<mathboard::Stroke> &strokes) {
  std::vector<mathboard::Stroke> intersecting_strokes;
  for (const auto &stroke : strokes) {
    // checks if rectangles are intersecting
    const cv::Rect2i &intersection =
        target_stroke.GetRect() & stroke.GetRect();
    if (intersection.area() != 0) {
      intersecting_strokes.push_back(stroke);
    }
  }
  return intersecting_strokes;
}

} // namespace mathboard

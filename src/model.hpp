#pragma once

// libs
// tensorflow-lite
#include "tensorflow/lite/core/model_builder.h"
#include "tensorflow/lite/interpreter.h"
// opencv
#include <opencv2/core/mat.hpp>

// std
#include <filesystem>
#include <memory>

namespace mathboard {

class Model {
public:
  Model(const std::filesystem::path &model_filename);
  // Predicts the label for the given input matrix (character).
  // The input matrix must meet the following conditions:
  // - It must be normalized
  // - It must be grayscaled
  // - It must be 28x28 in size
  // - It must have a data type of CV_32F
  // If the matrix represents a digit that is too narrow, the prediction may be
  // inaccurate.
  uint32_t Predict(cv::Mat input_mat) const;

private:
  std::unique_ptr<tflite::FlatBufferModel> m_Model{nullptr};
  std::unique_ptr<tflite::Interpreter> m_Interpreter{nullptr};
};
} // namespace mathboard
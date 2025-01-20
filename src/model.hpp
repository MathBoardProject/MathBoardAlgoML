#pragma once

// libs
// tensorflow-lite
#include "tensorflow/lite/interpreter.h"
// opencv
#include <opencv2/core/mat.hpp>

// std
#include <memory>

namespace mathboard {

class Model {
public:
  Model(const std::string &model_filename);
  uint32_t Predict(cv::Mat character) const;

private:
  std::unique_ptr<tflite::Interpreter> m_Interpreter;
};
} // namespace mathboard
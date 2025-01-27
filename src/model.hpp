#pragma once

// libs
// tensorflow-lite
#include "tensorflow/lite/core/model_builder.h"
#include "tensorflow/lite/interpreter.h"
// opencv
#include <filesystem>
#include <opencv2/core/mat.hpp>

namespace mathboard {

class Model {
public:
  Model(const std::filesystem::path &model_filename);
  uint32_t Predict(cv::Mat character) const;

private:
  std::unique_ptr<tflite::FlatBufferModel> m_Model{nullptr};
  std::unique_ptr<tflite::Interpreter> m_Interpreter{nullptr};
};

} // namespace mathboard

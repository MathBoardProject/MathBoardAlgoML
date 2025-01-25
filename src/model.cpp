// header
#include "model.hpp"

// libs
// tensorflow-lite
#include "spdlog/spdlog.h"
#include "tensorflow/lite/core/interpreter_builder.h"
#include "tensorflow/lite/examples/label_image/get_top_n.h"
#include "tensorflow/lite/kernels/register.h"

// std
#include <opencv2/core/hal/interface.h>
#include <vector>

namespace mathboard {
Model::Model(const std::filesystem::path &model_filename) {
  m_Model = tflite::FlatBufferModel::BuildFromFile(model_filename.c_str());
  if (m_Model == nullptr) {
    spdlog::error("[Model::Model]: Model loading failure\n");
  }
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*m_Model, resolver);
  builder(&m_Interpreter);
  if (m_Interpreter == nullptr) {
    spdlog::error("[Model::Model]: Interpreter loading failure\n");
  }
  m_Interpreter->AllocateTensors();
}
uint32_t Model::Predict(cv::Mat character) const {
  if(character.rows != 28 || character.cols != 28
    || character.channels() != 1 || character.type() != CV_32F) {
    spdlog::error("Wrong matrix format\n");
  }
  const float treshold = 0.1f;
  std::vector<std::pair<float, int>> top_results;

  memcpy(m_Interpreter->typed_input_tensor<float>(0), character.data,
          character.total() * character.elemSize());

  // inference
  m_Interpreter->Invoke();

  // get output
  auto output = m_Interpreter->outputs()[0];
  TfLiteIntArray *output_dims = m_Interpreter->tensor(output)->dims;
  auto output_size = output_dims->data[output_dims->size - 1];

  tflite::label_image::get_top_n<float>(
      m_Interpreter->typed_output_tensor<float>(0), output_size, 1, treshold,
      &top_results, kTfLiteFloat32);
  return top_results.front().second;
}
} // namespace mathboard
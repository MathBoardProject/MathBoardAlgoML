// header
#include "model.hpp"

// libs
// tensorflow-lite
#include "tensorflow/lite/core/interpreter_builder.h"
#include "tensorflow/lite/examples/label_image/get_top_n.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model_builder.h"

// std
#include <vector>

namespace mathboard {
Model::Model(const std::string &model_filename) {
  std::unique_ptr<tflite::FlatBufferModel> model =
      tflite::FlatBufferModel::BuildFromFile(model_filename.c_str());
  if (model == nullptr) {
  }
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*model, resolver);
  std::unique_ptr<tflite::Interpreter> interpreter;
  builder(&m_Interpreter);
  if (m_Interpreter == nullptr) {
  }
  m_Interpreter->AllocateTensors();
}
uint32_t Model::Predict(cv::Mat character) const {
  const float treshold = 0.1f;
  std::vector<std::pair<float, int>> top_results;

  memcpy(m_Interpreter->typed_input_tensor<float>(0), character.data,
         character.total() * character.elemSize());
  // inference
  m_Interpreter->Invoke();

  // get output
  int output = m_Interpreter->outputs()[0];
  TfLiteIntArray *output_dims = m_Interpreter->tensor(output)->dims;
  auto output_size = output_dims->data[output_dims->size - 1];

  tflite::label_image::get_top_n<float>(
      m_Interpreter->typed_output_tensor<float>(0), output_size, 1, treshold,
      &top_results, kTfLiteFloat32);
  return top_results.front().second;
}
} // namespace mathboard
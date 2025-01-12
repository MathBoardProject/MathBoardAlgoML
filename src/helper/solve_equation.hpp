#pragma once

// local
#include "../equation_solver.hpp"
#include "../image_processing.hpp"
#include "../opencv_helper.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>

// std
#include <string>

namespace mathboard {

void solveImage(const std::string &imagePath) {
  // Step 1: Preprocess the input image
  OpenCVHelper opencvHelper{};
  cv::Mat processedImage;
  opencvHelper.OpenFile(imagePath);
  processedImage = opencvHelper.GetFrame();
  processedImage = GrayScaleImage(processedImage);
  processedImage = BinarizeImage(processedImage);

  // Step 2: Recognize text (equation) using OCR
  std::string rawEquation = RecognizeText(processedImage);
  spdlog::info("[solveImage]: Raw OCR Output: {}\n", rawEquation);

  std::cout << "Raw OCR Output: " << rawEquation << std::endl;

  // Step 3: Sanitize the OCR output
  std::string sanitizedEquation = EquationSolver::SanitizeEquation(rawEquation);
  spdlog::info("[solveImage]: Sanitized equation: {}\n", sanitizedEquation);

  // Step 4: Solve the equation
  const std::string symbol = "X";
  std::unordered_map<std::string, std::vector<double>> solutions =
      EquationSolver::SolveFor(sanitizedEquation, {symbol});
  spdlog::info("Solutions: {}\n", solutions.at(symbol)[0]);
}

} // namespace mathboard
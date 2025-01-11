#pragma once

// std
#include <string>
#include <unordered_map>
#include <vector>

namespace mathboard {

class EquationSolver {
public:
  // Function to sanitize and validate the OCR output
  static std::string SanitizeEquation(const std::string &rawEquation);

  // Function to solve the equation
  static std::unordered_map<std::string, std::vector<double>>
  SolveFor(const std::string &equation,
           const std::vector<std::string> &symbols);
};

} // namespace mathboard

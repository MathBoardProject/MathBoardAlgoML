// header
#include "equation_solver.hpp"

// libs
// symengine
#include <symengine/expression.h>
#include <symengine/parser.h>
#include <symengine/solve.h>
// spdlog
#include <spdlog/spdlog.h>

namespace mathboard {

// Function to sanitize and validate the OCR output
std::string EquationSolver::SanitizeEquation(const std::string &rawEquation) {
  std::string sanitized;
  for (const char c : rawEquation) {
    if (std::isalnum(c) ||
        std::string("+-*/=().").find(c) != std::string::npos) {
      sanitized += c;
    }
  }

  // Remove any excess whitespace
  sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(), ::isspace),
                  sanitized.end());

  if (sanitized.empty()) {
    spdlog::error("[SanitizeEquation]: Sanitized equation is empty.\n");
  }

  // Transform equation to set it equal to zero
  size_t equalPos = sanitized.find('=');
  if (equalPos != std::string::npos) {
    std::string left = sanitized.substr(0, equalPos);
    std::string right = sanitized.substr(equalPos + 1);
    sanitized = left + "-(" + right + ")";
  }

  // Validate parentheses
  int openParentheses = std::count(sanitized.begin(), sanitized.end(), '(');
  int closeParentheses = std::count(sanitized.begin(), sanitized.end(), ')');
  if (openParentheses != closeParentheses) {
    spdlog::error(
        "[SanitizeEquation]: Mismatched parentheses in the equation.\n");
  }

  return sanitized;
}

// Function to solve the equation
std::unordered_map<std::string, std::vector<double>>
EquationSolver::SolveFor(const std::string &equation,
                         const std::vector<std::string> &symbols) {
  // Parse the equation into a symbolic expression
  SymEngine::Expression expr = SymEngine::parse(equation);

  // Store the results in a map of symbol to solutions relation
  std::unordered_map<std::string, std::vector<double>> results;

  // Solve for symbol
  for (const auto &symbol : symbols) {
    SymEngine::RCP<const SymEngine::Set> solutions =
        solve(expr, SymEngine::symbol(symbol));

    // Convert solutions to strings
    for (const auto &sol : solutions->get_args()) {
      results[symbol].push_back(std::stod(sol->__str__()));
    }
  }

  return results;
}

} // namespace mathboard

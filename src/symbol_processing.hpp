#pragma once

// libs
// opencv
#include <opencv2/core/types.hpp>

// std
#include <vector>

namespace mathboard {
class Symbol;
class Stroke;
class Model;

struct VectorHash {
  std::size_t operator()(const std::vector<Stroke> &vec) const;
};

// TODO:
// make this function more efficient by avoiding a creation of
// unique_combination vector every time smbdy use this function
std::vector<std::vector<Stroke>>
EvaluateCombinations(const std::vector<std::vector<Stroke>> &combinations,
                     const std::vector<std::vector<Symbol>> &symbol_groups);

Symbol GetBestSymbol(const Model &model,
                     const std::vector<std::vector<Stroke>> &combinations);

std::vector<std::vector<Symbol>>
GenerateSymbolGroups(const Model &model, const std::vector<Stroke> strokes);

bool Contains(const cv::Rect &a, const cv::Rect &b);

double ComputePredictionScore(const std::vector<double> &confidences,
                              int fully_contained_symbols);

std::vector<Symbol>
GetBestSymbolGroup(const std::vector<std::vector<Symbol>> &symbol_groups);
} // namespace mathboard
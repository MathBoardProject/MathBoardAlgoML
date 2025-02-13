// header
#include "symbol_processing.hpp"

// local
#include "model.hpp"
#include "stroke.hpp"
#include "symbol.hpp"

// std
#include <numeric>
#include <unordered_set>

namespace mathboard {
std::size_t VectorHash::operator()(const std::vector<Stroke> &vec) const {
  std::size_t hash = 0;
  for (const Stroke &stroke : vec) {
    hash ^= std::hash<std::size_t>{}(stroke.GetIndex()) + 0x9e3779b9 +
            (hash << 6) + (hash >> 2);
  }
  return hash;
}

std::vector<std::vector<Stroke>>
EvaluateCombinations(const std::vector<std::vector<Stroke>> &combinations,
                     const std::vector<std::vector<Symbol>> &symbol_groups) {
  std::vector<std::vector<Stroke>> unique_combinations;
  unique_combinations.reserve(combinations.size());
  // Store prediction indexes in an unordered_set for O(1) lookup
  std::unordered_set<std::vector<Stroke>, VectorHash> symbol_groups_set;
  for (const auto &prediction_set : symbol_groups) {
    for (const auto &prediction : prediction_set) {
      symbol_groups_set.insert(prediction.GetStrokes());
    }
  }

  for (auto it = combinations.begin(); it != combinations.end(); it++) {
    if (symbol_groups_set.contains(*it) == false) {
      unique_combinations.push_back(*it);
    }
  }
  return unique_combinations;
}

Symbol GetBestSymbol(const Model &model,
                     const std::vector<std::vector<Stroke>> &combinations) {
  constexpr float MIN_GROUP_CONFIDANCE = 95.0;
  Symbol best_symbol;
  double highest_group_confidence = MIN_GROUP_CONFIDANCE;
  double highest_single_confidence = 0.0;

  // search for best symbol
  for (std::size_t i = 0; i < combinations.size(); i++) {
    const cv::Mat symbol = CombineStrokes(combinations[i]);
    const cv::Mat MNIST_symbol = ResizeToMNISTFormat(symbol);
    const auto prediction = model.Predict(MNIST_symbol);
    // TODO:
    // add new parameters to consider while choosing best prediction
    // benefit the combinations containing multiple strokes
    if (combinations[i].size() > 1 &&
        prediction.first > highest_group_confidence) {
      best_symbol = Symbol(i, static_cast<char>('0' + prediction.second),
                           prediction.first, combinations[i]);
      highest_group_confidence = best_symbol.GetConfidence();
    } else if (prediction.first > highest_single_confidence &&
               highest_group_confidence == MIN_GROUP_CONFIDANCE) {
      best_symbol = Symbol(i, static_cast<char>('0' + prediction.second),
                           prediction.first, combinations[i]);
      highest_single_confidence = best_symbol.GetConfidence();
    }
  }
  return best_symbol;
}

std::vector<std::vector<Symbol>>
GenerateSymbolGroups(const Model &model, const std::vector<Stroke> strokes) {
  if (strokes.empty() == true) {
    spdlog::error("[GenerateSymbolGroups()]: strokes is empty");
  }

  std::vector<std::vector<Symbol>> symbol_groups;
  while (true) {
    std::vector<Stroke> remaining_strokes = strokes;
    std::vector<Symbol> symbol_group;
    std::size_t target_stroke_index = 0;
    bool has_unique_combinations_left = false;

    while (!remaining_strokes.empty() && target_stroke_index < strokes.size()) {
      const std::vector<Stroke> intersecting_strokes = FindIntersectingStrokes(
          strokes[target_stroke_index], remaining_strokes);
      if (intersecting_strokes.empty()) {
        target_stroke_index++;
        continue;
      }
      const std::vector<std::vector<Stroke>> combinations =
          GenerateCombinations(intersecting_strokes);

      auto unique_combinations =
          EvaluateCombinations(combinations, symbol_groups);

      Symbol best_symbol;
      if (unique_combinations.empty()) {
        best_symbol = GetBestSymbol(model, combinations);
        // delete used strokes from remaining_strokes
        for (const auto &stroke : combinations[best_symbol.GetIndex()]) {
          remaining_strokes.erase(std::remove(remaining_strokes.begin(),
                                              remaining_strokes.end(), stroke));
        }
      } else {
        best_symbol = GetBestSymbol(model, unique_combinations);
        has_unique_combinations_left = true;
        target_stroke_index++;
        // delete used strokes from remaining_strokes
        for (const auto &stroke : unique_combinations[best_symbol.GetIndex()]) {
          remaining_strokes.erase(std::remove(remaining_strokes.begin(),
                                              remaining_strokes.end(), stroke));
        }
      }
      symbol_group.push_back(best_symbol);
    }
    if (!has_unique_combinations_left) {
      break;
    }
    symbol_groups.push_back(symbol_group);
  }
  return symbol_groups;
}

bool Contains(const cv::Rect &a, const cv::Rect &b) {
  if ((a.contains(b.tl()) && a.contains(b.br())) ||
      (b.contains(a.tl()) && b.contains(a.br()))) {
    return true;
  }
  return false;
}

double ComputePredictionScore(const std::vector<double> &confidences,
                              int fully_contained_symbols) {
  if (confidences.empty()) {
    spdlog::error("[ComputePredictionScore]: Empty confidance vector");
    return -1.0;
  }

  const double sum =
      std::accumulate(confidences.begin(), confidences.end(), 0.0);
  const double mean_confidence = sum / confidences.size();
  const double min_confidence =
      *std::min_element(confidences.begin(), confidences.end());

  // Compute variance
  double variance = 0.0;
  for (double c : confidences) {
    variance += (c - mean_confidence) * (c - mean_confidence);
  }
  variance /= confidences.size();

  // Weight for average confidence
  const double w1 = 1.0;
  // Weight for min confidence
  const double w2 = 0.5;
  // Weight for variance
  const double w3 = 0.3;
  // Weight for number of fully contained symbols in other symbols
  const double w4 = 0.1;

  return (w1 * mean_confidence) + (w2 * min_confidence) - (w3 * variance) -
         (w4 * fully_contained_symbols);
}

std::vector<Symbol>
GetBestSymbolGroup(const std::vector<std::vector<Symbol>> &symbol_groups) {
  std::vector<Symbol> best_symbol_group;
  double best_symbol_group_score = -INFINITY;
  for (const auto &symbol_group : symbol_groups) {
    std::vector<double> symbol_group_confidances;
    std::vector<cv::Rect> symbol_group_rects;
    symbol_group_confidances.reserve(symbol_group.size());
    symbol_group_rects.reserve(symbol_group.size());
    for (const auto &symbol : symbol_group) {
      symbol_group_confidances.push_back(symbol.GetConfidence());
      symbol_group_rects.push_back(symbol.GetRect());
      const std::vector<mathboard::Stroke> &symbol_strokes =
          symbol.GetStrokes();
    }
    int fully_contained_symbols = 0;
    for (const cv::Rect &rectA : symbol_group_rects) {
      for (const cv::Rect &rectB : symbol_group_rects) {
        if (rectA == rectB) {
          continue;
        }
        if (Contains(rectA, rectB)) {
          fully_contained_symbols++;
        }
      }
    }
    const double symbol_group_score = ComputePredictionScore(
        symbol_group_confidances, fully_contained_symbols);
    if (symbol_group_score > best_symbol_group_score) {
      best_symbol_group = symbol_group;
      best_symbol_group_score = symbol_group_score;
    }
  }
  return best_symbol_group;
}
} // namespace mathboard
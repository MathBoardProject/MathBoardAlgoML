#pragma once

// local
#include "image_processing.hpp"
#include "model.hpp"
#include "opencv_helper.hpp"
#include "prediction.hpp"
#include "stroke.hpp"
#include "unix_socket_server/unix_socket_server.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>
// json
#include <nlohmann/json.hpp>
// opencv
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

// std
#include <string>
#include <utility>
#include <vector>

namespace mathboard {

struct VectorHash {
  std::size_t operator()(const std::vector<Stroke> &vec) const {
    std::size_t hash = 0;
    for (const Stroke &stroke : vec) {
      hash ^= std::hash<std::size_t>{}(stroke.GetIndex()) + 0x9e3779b9 +
              (hash << 6) + (hash >> 2);
    }
    return hash;
  }
};

[[nodiscard]] inline std::vector<std::vector<Stroke>>
EvaluateCombinations(const std::vector<std::vector<Stroke>> &combinations,
                     const std::vector<std::vector<Prediction>> &predictions) {
  std::vector<std::vector<Stroke>> unique_combinations;
  unique_combinations.reserve(combinations.size());
  // Store prediction indexes in an unordered_set for O(1) lookup
  std::unordered_set<std::vector<Stroke>, VectorHash> predictions_set;
  for (const auto &prediction_set : predictions) {
    for (const auto &prediction : prediction_set) {
      predictions_set.insert(prediction.strokes);
    }
  }

  for (auto it = combinations.begin(); it != combinations.end(); it++) {
    if (predictions_set.contains(*it) == false) {
      unique_combinations.push_back(*it);
    }
  }
  return unique_combinations;
}

[[nodiscard]] inline Prediction
GetBestPrediction(const Model &model,
                  const std::vector<std::vector<Stroke>> &combinations) {
  constexpr float MIN_GROUP_CONFIDANCE = 95.0;
  Prediction best_prediction;
  double highest_group_confidance = MIN_GROUP_CONFIDANCE;
  double highest_single_confidance = 0.0;
  // search for best prediction of symbol with remaining_strokes[0]
  for (std::size_t i = 0; i < combinations.size(); i++) {
    const cv::Mat symbol = CombineStrokes(combinations[i]);
    const cv::Mat MNIST_symbol = ResizeToMNISTFormat(symbol);
    const auto prediction = model.Predict(MNIST_symbol);
    // TODO:
    // add new parameters to consider while choosing best prediction
    // benefit the combinations containing multiple strokes
    if (combinations[i].size() > 1 &&
        prediction.first > highest_group_confidance) {
      best_prediction =
          Prediction(prediction.first, prediction.second, i, combinations[i]);
      highest_group_confidance = best_prediction.confidance;
    } else if (prediction.first > highest_single_confidance &&
               highest_group_confidance == MIN_GROUP_CONFIDANCE) {
      best_prediction =
          Prediction(prediction.first, prediction.second, i, combinations[i]);
      highest_single_confidance = best_prediction.confidance;
    }
  }
  return best_prediction;
}

/*
FORMAT:
```
{
  strokes: [
    {
      id: number;
      boardId: number;
      path: string;     // SVG file path
      x: number;        // in pixels
      y: number;        // in pixels
    },
    {
      ...
    }
  ]
}
```
*/

void Daemon() {
  std::unique_ptr<UnixSocketServer> server(UnixSocketServer::Instance());
  OpenCVHelper opencvHelper{};

  bool running = true;

  if (!server->Init("socket.sock")) {
    spdlog::error("[Daemon] - Failed to initialize the Unix Socket Server.\n");
    return;
  }

  server->Listen();

  spdlog::info("[Daemon] - Server is listening.\n");

  void *client_addr;
  int client_fd;

  bool connected = false;

  // Loop to handle request
  while (running) {
    if (!connected) {
      if (!server->Accept(client_fd, &client_addr)) {
        spdlog::error("[Daemon] - Failed to accept the connection.\n");
        continue;
      }

      connected = true;
      spdlog::info("[Daemon] - Client connected.\n");
    }

    if (connected) {
      std::vector<unsigned char> buffer(1024);

      server->Read(client_fd, buffer);

      if (buffer.size() == 0) {
        continue;
      }

      std::string stringData = std::string(buffer.begin(), buffer.end());
      nlohmann::json jsonData = nlohmann::json::parse(stringData);
      auto &strokesData = jsonData["strokes"];

      std::vector<Stroke> stroke_vector;
      stroke_vector.reserve(strokesData.size());

      for (std::size_t i = 0; i != strokesData.size(); i++) {
        auto &strokeData = strokesData[i];
        cv::Mat processedImage;
        opencvHelper.OpenFile(strokeData["path"]);
        processedImage = opencvHelper.GetFrame();
        processedImage = GrayScaleImage(processedImage);

        stroke_vector.emplace_back(strokeData["id"], strokeData["x"],
                                   strokeData["y"], processedImage);
      }
      Model model(
          "/home/projects/MathBoardAlgoML/models/converted_model.tflite");

      std::vector<std::vector<Prediction>> all_predictions;
      while (true) {
        std::vector<Stroke> remaining_strokes = stroke_vector;
        std::vector<Prediction> predictions;
        std::size_t target_stroke_index = 0;
        bool has_unique_combinations_left = false;
        while (!remaining_strokes.empty() &&
               target_stroke_index < stroke_vector.size()) {
          const std::vector<Stroke> intersecting_strokes =
              FindIntersectingStrokes(stroke_vector[target_stroke_index],
                                      remaining_strokes);
          if (intersecting_strokes.empty()) {
            target_stroke_index++;
            continue;
          }
          const std::vector<std::vector<Stroke>> combinations =
              GenerateCombinations(intersecting_strokes);

          auto unique_combinations =
              EvaluateCombinations(combinations, all_predictions);

          Prediction best_prediction;
          if (unique_combinations.empty()) {
            best_prediction = GetBestPrediction(model, combinations);
            // delete used strokes from remaining_strokes
            for (const auto &stroke : combinations[best_prediction.index]) {
              remaining_strokes.erase(std::remove(
                  remaining_strokes.begin(), remaining_strokes.end(), stroke));
            }
          } else {
            best_prediction = GetBestPrediction(model, unique_combinations);
            has_unique_combinations_left = true;
            target_stroke_index++;
            // delete used strokes from remaining_strokes
            for (const auto &stroke :
                 unique_combinations[best_prediction.index]) {
              remaining_strokes.erase(std::remove(
                  remaining_strokes.begin(), remaining_strokes.end(), stroke));
            }
          }
          predictions.push_back(best_prediction);
        }
        if (!has_unique_combinations_left) {
          break;
        }
        all_predictions.push_back(predictions);
      }
    }
  }
}
} // namespace mathboard

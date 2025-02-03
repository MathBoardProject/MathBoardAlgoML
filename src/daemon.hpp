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
#include <vector>

namespace mathboard {

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

      std::vector<Stroke> strokeVector;
      strokeVector.reserve(strokesData.size());

      for (std::size_t i = 0; i != strokesData.size(); i++) {
        auto &strokeData = strokesData[i];
        cv::Mat processedImage;
        opencvHelper.OpenFile(strokeData["path"]);
        processedImage = opencvHelper.GetFrame();
        processedImage = GrayScaleImage(processedImage);

        strokeVector.emplace_back(strokeData["id"], strokeData["x"],
                                  strokeData["y"], processedImage);
      }
      Model model(
          "/home/projects/MathBoardAlgoML/models/converted_model.tflite");

      std::vector<Stroke> remaining_strokes = strokeVector;
      std::vector<cv::Mat> results;

      constexpr double MIN_GROUP_CONFIDANCE = 0.95;
      while (!remaining_strokes.empty()) {
        std::vector<Stroke> intersecting_strokes =
            FindIntersectingStrokes(remaining_strokes[0], remaining_strokes);

        const std::vector<std::vector<Stroke>> combinations =
            GenerateCombinations(intersecting_strokes);
        Prediction best_prediction;
        double highest_group_confidance = MIN_GROUP_CONFIDANCE;
        double highest_single_confidance = 0.0;

        // search for best prediction of symbol with remaining_strokes[0]
        for(std::size_t i = 0; i < combinations.size(); i++) {
          const cv::Mat symbol = CombineStrokes(combinations[i]);
          const cv::Mat MNIST_symbol = ResizeToMNISTFormat(symbol);
          const auto prediction = model.Predict(MNIST_symbol);
          // benefit the combinations containing multiple strokes
          if (combinations[i].size() > 1 &&
              prediction.first > highest_group_confidance) {
            best_prediction.confidance = prediction.first;
            best_prediction.guessed_number = prediction.second;
            best_prediction.symbol_index = i;
            highest_group_confidance = best_prediction.confidance;
          } else if (prediction.first > highest_single_confidance &&
                     highest_group_confidance == MIN_GROUP_CONFIDANCE) {
            best_prediction.confidance = prediction.first;
            best_prediction.guessed_number = prediction.second;
            best_prediction.symbol_index = i;
            highest_single_confidance = best_prediction.confidance;
          }
        }
        // delete used strokes from remaining_strokes
        for (const auto &stroke : combinations[best_prediction.symbol_index]) {
          remaining_strokes.erase(std::remove(remaining_strokes.begin(), remaining_strokes.end(), stroke));
        }
        cv::Mat best_prediction_matrix =
            CombineStrokes(combinations[best_prediction.symbol_index]);
        results.push_back(best_prediction_matrix);
      }
    }
  }
}
} // namespace mathboard

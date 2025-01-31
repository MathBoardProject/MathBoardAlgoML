#pragma once

// local
#include "image_processing.hpp"
#include "model.hpp"
#include "opencv_helper.hpp"
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
#include <cstddef>
#include <algorithm>
#include <string>
#include <iterator>
#include <vector>

namespace mathboard {

struct Prediction {
  Prediction(double confidance, int number, std::size_t symbol_index):
    confidance(confidance), guessed_number(number), symbol_index(symbol_index) {}
  double confidance{0.0};
  int guessed_number{0};
  std::size_t symbol_index;
};

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
      mathboard::Model model(
      "/home/projects/MathBoardAlgoML/models/converted_model.tflite");
  
      std::vector<mathboard::Stroke> remaining_strokes = strokeVector;
      std::vector<std::string> results;

      while (!remaining_strokes.empty()) {
        spdlog::info("remaining_strokes[0] index {}",remaining_strokes[0].GetIndex());
        std::vector<mathboard::Stroke> stroke_group =
            mathboard::FindIntersectingStrokes(remaining_strokes[0],
                                               remaining_strokes);

        const std::vector<std::vector<mathboard::Stroke>> combinations =
            mathboard::GenerateCombinations(stroke_group);
        std::vector<mathboard::Prediction> predictions;
        // stroke combination index if the combination is bigger than one element and the model's confidence level is above 99%
        std::size_t idx = combinations.size();
        predictions.reserve(combinations.size());
        // look for the best prediction in given combination
        for(std::size_t i = 0; i < combinations.size(); i++) {
          const cv::Mat symbol = mathboard::CombineStrokes(combinations[i]);
          const cv::Mat MNIST_symbol = mathboard::ResizeToMNISTFormat(symbol);
          auto prediction = model.Predict(MNIST_symbol);
          predictions.emplace_back(prediction.first, prediction.second, i);
          // it is likely that this combination might be the right answear but it is temporary solution
          if(combinations[i].size() > 1 && predictions.back().confidance > 0.99) {
            idx = i;
          }
        }
        std::size_t best_prediction_index = 0;
        if(idx != combinations.size()) {
          best_prediction_index = idx;
        } else {
          const auto best_prediction = std::max_element(
            predictions.begin(), predictions.end(), [](const auto &a, const auto &b){
              return a.confidance < b.confidance;
            });
          best_prediction_index = std::distance(predictions.begin(), best_prediction);
        }

        cv::Mat predicted_symbol = mathboard::CombineStrokes(combinations[best_prediction_index]);
        cv::namedWindow("prediction", cv::WINDOW_AUTOSIZE);
        cv::imshow("prediction", predicted_symbol);
        cv::waitKey(0);
        for (const auto &stroke : combinations[best_prediction_index]) {
          remaining_strokes.erase(std::remove(remaining_strokes.begin(), remaining_strokes.end(), stroke));
        }
        results.emplace_back(std::to_string(predictions[best_prediction_index].guessed_number));
      }
      for(std::size_t i = 0; i < results.size(); i++) {
        spdlog::info("{}. result: {}", i, results[i]);
      }
  }
  }
}
} // namespace mathboard

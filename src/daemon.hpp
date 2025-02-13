#pragma once

// local
#include "image_processing.hpp"
#include "model.hpp"
#include "opencv_helper.hpp"
#include "symbol.hpp"
#include "stroke.hpp"
#include "symbol_processing.hpp"
#include "unix_socket_server/unix_socket_server.hpp"

// libs
// spdlog
#include <spdlog/spdlog.h>
// json
#include <nlohmann/json.hpp>

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
      const auto symbol_groups = GenerateSymbolGroups(model, stroke_vector);
      const auto symbol_group = GetBestSymbolGroup(symbol_groups);
    }
  }
}
} // namespace mathboard

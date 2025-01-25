// header
#include "opencv_helper.hpp"

// lib
// spdlog
#include <spdlog/spdlog.h>

// std
#include <filesystem>

namespace mathboard {

// Open file
void OpenCVHelper::OpenFile(const std::filesystem::path &file) {
  // Stop the rendering while new file isn't loaded yet
  m_ShouldRender = false;

  // Check if the file is a video or an image
  if (IsImageExtension(file)) {
    // Make sure you don't change frame while it's being used
    // somewhere else
    std::lock_guard<std::mutex> lockFrame(m_MutexFrame);

    // Load image
    m_Frame = cv::imread(file.string());

    if (m_Frame.empty()) {
      spdlog::error("[OpenCVHelper::OpenFile]: Could not open the image.\n");
      return;
    }

    m_IsVideo = false;
  } else {

    {
      // Make sure you don't change video capture while it's being used
      // somewhere else
      std::lock_guard<std::mutex> lockVideoCapture(m_MutexVideoCapture);

      // Open the video file
      m_VideoCapture.open(file.string());
    }

    if (!m_VideoCapture.isOpened()) {
      spdlog::error("[OpenCVHelper::OpenFile]: Could not open the video.\n");
      return;
    }

    // If the image is loaded using video capture (ffmpeg) instead of imread
    // (has 0 frames), still treat it as an image
    if (GetTotalFrameCount() == 0) {
      // Make sure you don't change frame while it's being used
      // somewhere else
      std::lock_guard<std::mutex> lockFrame(m_MutexFrame);

      m_VideoCapture >> m_Frame;
      m_IsVideo = false;
    } else {
      m_IsVideo = true;
    }
  }

  // Done loading the file
  m_ShouldRender = true;
}

// Render the whole video capture
void OpenCVHelper::RenderVideo() {
  // Stop the rendering while new file isn't loaded yet or when finished
  while (GetCurrentFrameIndex() <= GetTotalFrameCount() && m_ShouldRender) {
    {
      // Make sure you don't change m_Frame while it's being used somewhere else
      // Make sure that the video capture doesn't change while it's being
      // accessed here
      std::lock_guard<std::mutex> lockVideoCapture(m_MutexVideoCapture);
      std::lock_guard<std::mutex> lockFrame(m_MutexFrame);

      // Read next frame from the video
      m_VideoCapture >> m_Frame;
    }

    // cv::CAP_PROP_POS_FRAMES begins counting from 1
    // (0 means frame is not loaded)
    ConvertFrame(static_cast<std::int32_t>(GetCurrentFrameIndex()) - 1);
  }
}

// Convert a frame
void OpenCVHelper::ConvertFrame(const std::uint32_t index) {
  // Make sure that the frame doesn't change while it's being accessed here
  std::lock_guard<std::mutex> lockFrame(m_MutexFrame);

  if (m_Frame.empty() || m_Frame.cols == 0 || m_Frame.rows == 0) {
    return;
  }

  // Calculate the aspect ratio of the original frame
  const float aspectRatio =
      static_cast<float>(m_Frame.rows) / static_cast<float>(m_Frame.cols);

  // Calculate the block size based on desired size
  const std::uint32_t blockSizeX = std::max(
      1U, m_Frame.cols / static_cast<std::uint32_t>(m_Size / aspectRatio));

  const std::uint32_t blockSizeY = std::max(1U, m_Frame.rows / m_Size);

  // Calculate the number of blocks.
  const std::uint32_t numBlocksX = m_Frame.cols / blockSizeX;
  const std::uint32_t numBlocksY = m_Frame.rows / blockSizeY;

  for (std::uint32_t i = 0; i < numBlocksX; i++) {
    for (std::uint32_t j = 0; j < numBlocksY; j++) {
      std::uint32_t sum_r = 0;
      std::uint32_t sum_g = 0;
      std::uint32_t sum_b = 0;

      // Calculate the average for each block
      for (std::uint32_t bi = 0; bi < blockSizeX; ++bi) {
        for (std::uint32_t bj = 0; bj < blockSizeY; ++bj) {
          const cv::Vec3b pixel =
              m_Frame.at<cv::Vec3b>(j * blockSizeY + bj, i * blockSizeX + bi);

          // OpenCV has BGR color format
          const std::uint8_t b = pixel[0];
          const std::uint8_t g = pixel[1];
          const std::uint8_t r = pixel[2];

          sum_r += r;
          sum_g += g;
          sum_b += b;
        }
      }

      // Calculate average luminance of the frame region
      const unsigned long avg =
          (sum_r + sum_g + sum_b) / (3 * blockSizeX * blockSizeY);

      // TODO
      // Do something with the average color
    }
  }
}

} // namespace mathboard
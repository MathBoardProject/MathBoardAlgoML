#pragma once

// lib
// OpenCV
#include <opencv2/opencv.hpp>

// std
#include <cstdint>
#include <mutex>
#include <filesystem>

namespace mathboard {

class OpenCVHelper {
public:
  OpenCVHelper() = default;

  // Ctor that opens the file
  OpenCVHelper(const std::filesystem::path &file) { OpenFile(file); }

  // Release the video
  ~OpenCVHelper() { m_VideoCapture.release(); }

  // Open file
  void OpenFile(const std::filesystem::path &file);

  // Render the whole video capture
  void RenderVideo();

  // Convert a frame
  void ConvertFrame(const std::uint32_t index);

  // Get framerate
  std::uint32_t GetFramerate() const {
    // Return framerate or 1 for images
    return std::max(
        1U, static_cast<std::uint32_t>(m_VideoCapture.get(cv::CAP_PROP_FPS)));
  }

  // Return currently accessed frame index
  std::uint32_t GetCurrentFrameIndex() const {
    return m_VideoCapture.get(cv::CAP_PROP_POS_FRAMES);
  }

  // Return frame count of the currently loaded media
  std::uint32_t GetTotalFrameCount() const {
    return m_VideoCapture.get(cv::CAP_PROP_FRAME_COUNT);
  }

  cv::Mat GetFrame() const { return m_Frame; }

  // Get whether a video or an image is loaded
  bool IsVideo() const { return m_IsVideo; }

  // Check the file extension to determine whether its an image
  bool IsImageExtension(const std::filesystem::path &filename) const {
    return filename.extension() == ".jpg" || filename.extension() == ".jpeg" ||
           filename.extension() == ".png" || filename.extension() == ".bmp";
    // SVGs are loaded using FFmpeg
  }

  // Set blocksize
  void SetSize(const std::uint32_t size) { m_Size = size; }

  // Get blocksize
  std::uint32_t GetSize() const { return m_Size; }

  // Set whether to continue rendering
  void ContinueRendring(const bool should_render) {
    m_ShouldRender = should_render;
  }

  // Set current frame index
  void SetCurrentFrameIndex(const std::uint32_t index) {
    m_VideoCapture.set(cv::CAP_PROP_POS_FRAMES, index);
  }

private: // Attributes
  // Loaded video or image
  bool m_IsVideo = false;

  // Should you contine rendering?
  bool m_ShouldRender = false;

  // Blocksize
  std::uint32_t m_Size = 1;

  // Vidoe capture
  cv::VideoCapture m_VideoCapture{};

  // Video frame or image
  cv::Mat m_Frame{};

  // Make sure that the video capture does't get updated in two places
  // at the same time
  std::mutex m_MutexVideoCapture{};

  // Make sure that the frame does't get updated in two places
  // at the same time
  std::mutex m_MutexFrame{};
};

} // namespace mathboard
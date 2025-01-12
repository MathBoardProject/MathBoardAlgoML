// local
#include "grid.hpp"
#include "stroke.hpp"

// opencv
#include <opencv2/opencv.hpp>

// std
#include <filesystem>

namespace mathboard {
// take path to svg file and transform it into pixel representation using
// cv::Mat
cv::Mat RasterizeImage(const std::filesystem::path &filename);

// `input_array` has to be grayscale
// crop image to tightly fit symbol on it
cv::Mat CropImageToSymbol(const cv::Mat &input_mat);

// returns instance of Grid class with all images put on
// their positions ready to further interpreatation
// it sets grid cell size and boundaries of it
// automaticaly
// Return grascaled version of input image
cv::Mat GrayScaleImage(const cv::Mat &input_mat);

// Return image binarized using threshold.
cv::Mat BinarizeImage(const cv::Mat &input_mat);

// Returns image string
std::string RecognizeText(const cv::Mat &img);

inline Grid<mathboard::Stroke>
PlaceOnGrid(std::vector<mathboard::Stroke> &strokes) {
  // calculate boundaries of grid
  cv::Point2f bot_right_corner{0, 0};
  cv::Point2f top_left_corner{INFINITY, INFINITY};
  for (std::size_t i = 0; i < strokes.size(); i++) {
    const cv::Point2f line_pos = strokes[i].GetPosition();
    const cv::Rect line_BB = strokes[i].GetBoundingBox();

    if (bot_right_corner.x < line_pos.x + line_BB.width) {
      bot_right_corner.x = line_pos.x + line_BB.width;
    }
    if (top_left_corner.x > line_pos.x) {
      top_left_corner.x = line_pos.x;
    }
    if (bot_right_corner.y < line_pos.y + line_BB.height) {
      bot_right_corner.y = line_pos.y + line_BB.height;
    }
    if (top_left_corner.y > line_pos.y) {
      top_left_corner.y = line_pos.y;
    }
  }

  // calculate avrage size of stroke
  cv::Size2f avrage_stroke_size{0.0f, 0.0f};
  for (std::size_t i = 0; i < strokes.size(); i++) {
    avrage_stroke_size.width += strokes[i].GetWidth();
    avrage_stroke_size.height += strokes[i].GetHeight();
  }
  avrage_stroke_size.width /= static_cast<float>(strokes.size());
  avrage_stroke_size.height /= static_cast<float>(strokes.size());

  // create grid
  Grid<mathboard::Stroke> grid(top_left_corner, bot_right_corner,
                               avrage_stroke_size);
  for (std::size_t i = 0; i < strokes.size(); i++) {
    grid.Insert(&strokes[i]);
  }
  return grid;
}
} // namespace mathboard
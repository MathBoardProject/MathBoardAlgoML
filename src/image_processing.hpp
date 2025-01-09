// local
#include "stroke.hpp"

// opencv
#include <opencv2/opencv.hpp>

// std
#include <filesystem>

namespace mathboard {
class Grid;
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
Grid PlaceOnGrid(std::vector<mathboard::Stroke> &strokes);
} // namespace mathboard
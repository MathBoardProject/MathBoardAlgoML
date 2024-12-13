#include <filesystem>
#include <opencv2/opencv.hpp>

namespace mathboard {
// take path to svg file and transform it into pixel representation using
// cv::Mat
bool RasterizeImage(const std::filesystem::path &filename, cv::Mat &output_mat);

// `input_array` has to be grayscale
// crop image to tightly fit symbol on it
void CropImageToSymbol(const cv::Mat &input_mat, cv::Mat &output_mat);
} // namespace mathboard
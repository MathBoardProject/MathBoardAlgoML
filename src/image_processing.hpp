// opencv
#include <opencv2/opencv.hpp>

// std
#include <filesystem>

namespace mathboard {
class Grid;
// take path to svg file and transform it into pixel representation using
// cv::Mat
bool RasterizeImage(const std::filesystem::path &filename, cv::Mat &output_mat);

// `input_array` has to be grayscale
// crop image to tightly fit symbol on it
cv::Mat CropImageToSymbol(const cv::Mat &input_mat);

// returns instance of Grid class with all images put on
// their positions ready to further interpreatation
// it sets grid cell size and boundaries of it
// automaticaly
Grid PlceOnGrid(const std::vector<cv::Mat> &grayscale_images,
                const std::vector<cv::Point2f> &images_positions);

// Return grascaled version of input image
cv::Mat GrayScaleImage(const cv::Mat &input_mat);

// Return image binarized using threshold.
cv::Mat BinarizeImage(const cv::Mat &input_mat);

// Returns image string
std::string RecognizeText(const cv::Mat &img);

} // namespace mathboard
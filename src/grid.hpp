#pragma once

// local
#include "grid.hpp"
#include "stroke.hpp"

// opencv
#include <opencv2/opencv.hpp>

// std
#include <list>
#include <set>
#include <vector>

namespace mathboard {
// The Grid class handles broad-phase collision detection by dividing space into
// cells, each containing potential lines intersection.
class Grid {
public:
  // Constructor that sets up a grid covering a specified area,
  // defined by the top-left and bottom-right corners, with each cell having the
  // given size.
  Grid(const cv::Point2f &top_left_corner, const cv::Point2f &bot_right_corner,
       const cv::Size &cell_size);

public:
  // Inserts all lines into grid in appropriate place in space
  void InsertObjects(std::vector<Stroke> &lines);
  // Updates the grid with the positions of the objects.
  // It places each object's ID in the appropriate grid cell based on its
  // position.
  std::list<std::pair<int, int>> GetCollisionPairs() const;

private:
  // Checks if a pair of objects has already been checked for collisions.
  bool HasBeenChecked(std::unordered_multimap<int, int> &checked_pairs,
                      std::pair<int, int> pair) const;

private:
  cv::Point2f m_TopLeftCorner{0.0f, 0.0f};
  cv::Point2f m_BotRightCorner{100.0f, 100.0f};
  cv::Size2f m_CellSize{10.0f, 10.0f};
  // number of rows in grid
  std::uint32_t m_Rows{10};
  // number of columns in grid
  std::uint32_t m_Columns{10};
  std::vector<std::vector<std::vector<std::uint32_t>>> m_Grid;
};
} // namespace mathboard
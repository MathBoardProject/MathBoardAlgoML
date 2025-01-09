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
// The Grid class handles broad-phase intersection detection by dividing space
// into cells, each containing potential stroke intersection. Grid as a class
// doesn't own pointers to objects which stores.
class Grid {
public:
  // Constructor that sets up a grid covering a specified area,
  // defined by the top-left and bottom-right corners, with each cell having the
  // given size.
  Grid(const cv::Point2f &top_left_corner, const cv::Point2f &bot_right_corner,
       const cv::Size2i &cell_size);

  // Insert stroke to grid
  void InsertStroke(Stroke *stroke);
  // Returns list of pairs of objects inside the same grid cell.
  std::list<std::pair<Stroke *, Stroke *>> GetIntersectionPairs() const;

  // Clears all strokes inside all cells of the Grid.
  void Clear();

private:
  // Checks if a pair of objects has already been checked for collisions.
  bool
  HasBeenChecked(std::unordered_multimap<Stroke *, Stroke *> &checked_pairs,
                 std::pair<Stroke *, Stroke *> pair) const;

  cv::Point2f m_TopLeftCorner{0.0f, 0.0f};
  cv::Point2f m_BotRightCorner{0.0f, 0.0f};
  cv::Size2i m_CellSize{0, 0};
  std::uint32_t m_Rows{0};
  std::uint32_t m_Columns{0};
  std::vector<std::vector<Stroke *>> m_Grid;
};
} // namespace mathboard
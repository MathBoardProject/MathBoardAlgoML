// local
#include "grid.hpp"
#include "common.hpp"

// lib
// spdlog
#include <spdlog/spdlog.h>

// std
#include <fstream>

namespace mathboard {

Grid::Grid(const cv::Point2f &top_left_corner,
           const cv::Point2f &bot_right_corner, const cv::Size2i &cell_size)
    : m_TopLeftCorner(top_left_corner), m_BotRightCorner(bot_right_corner),
      m_CellSize(cell_size),
      m_Rows((m_BotRightCorner.y - m_TopLeftCorner.y) / m_CellSize.height),
      m_Columns((m_BotRightCorner.x - m_TopLeftCorner.x) / m_CellSize.width) {

  if (m_CellSize.width <= 0 || m_CellSize.height <= 0) {
    spdlog::error("[Grid::Grid]: m_CellSize width and height are {}, {} "
                  "instead of a positive number\n",
                  m_CellSize.width, m_CellSize.height);
  }

  m_Grid.resize(m_Rows * m_Columns);
}

void Grid::InsertStroke(Stroke *stroke) {
  const cv::Rect bounding_box = stroke->GetBoundingBox();
  cv::Point2f object_min = stroke->GetPosition();
  cv::Point2f object_max =
      cv::Point2f{stroke->GetPosition().x + bounding_box.width,
                  stroke->GetPosition().y + bounding_box.height};

  // calculating position of vertices in grid
  // decrese width and height because containers are 0 index based
  int body_min_x = static_cast<int>(
      std::floor((object_min.x - m_TopLeftCorner.x) / m_CellSize.width));
  body_min_x = std::clamp(body_min_x, 0, static_cast<int>(m_Rows - 1));

  int body_max_x = static_cast<int>(
      std::floor((object_max.x - m_TopLeftCorner.x) / m_CellSize.width));
  body_max_x = std::clamp(body_max_x, 0, static_cast<int>(m_Rows - 1));

  int body_min_y = static_cast<int>(
      std::floor((object_min.y - m_TopLeftCorner.y) / m_CellSize.height));
  body_min_y = std::clamp(body_min_y, 0, static_cast<int>(m_Columns - 1));

  int body_max_y = static_cast<int>(
      std::floor((object_max.y - m_TopLeftCorner.y)) / m_CellSize.height);
  body_max_y = std::clamp(body_max_y, 0, static_cast<int>(m_Columns - 1));

  for (std::ptrdiff_t x = body_min_x; x <= body_max_x; x++) {
    for (std::ptrdiff_t y = body_min_y; y <= body_max_y; y++) {
      m_Grid[x + m_Rows * y].push_back(stroke);
    }
  }
}

std::list<std::pair<Stroke *, Stroke *>> Grid::GetIntersectionPairs() const {
  std::unordered_multimap<Stroke *, Stroke *> checked_pairs;
  std::list<std::pair<Stroke *, Stroke *>> pairs;

  for (std::size_t i = 0; i < m_Grid.size(); i++) {
    // no possible intersection in this cell
    if (m_Grid[i].size() <= 1) {
      continue;
    }
    for (Stroke *element_A : m_Grid[i]) {
      for (Stroke *element_B : m_Grid[i]) {
        if (element_A == element_B) {
          break;
        }
        std::pair<Stroke *, Stroke *> pair;
        if (element_A->GetIndex() < element_B->GetIndex()) {
          pair = std::make_pair(element_A, element_B);
        } else {
          pair = std::make_pair(element_B, element_A);
        }
        if (!HasBeenChecked(checked_pairs, pair)) {
          pairs.push_back(pair);
          checked_pairs.insert(pair);
        }
      }
    }
  }
  return pairs;
}

void Grid::Clear() {
  for (auto &cell : m_Grid) {
    cell.clear();
  }
}

bool Grid::HasBeenChecked(
    std::unordered_multimap<Stroke *, Stroke *> &checked_pairs,
    std::pair<Stroke *, Stroke *> pair) const {
  auto [first, second] = checked_pairs.equal_range(pair.first);
  for (auto &i = first; i != second; ++i) {
    if (i->second == pair.second) {
      return true;
    }
  }
  return false;
}

} // namespace mathboard
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

  m_Grid.resize(m_Rows);
  for (std::size_t i = 0; i < m_Grid.size(); i++) {
    m_Grid[i].resize(m_Columns);
  }
}

void Grid::InsertObjects(std::vector<Stroke> &lines) {
  for (std::size_t i = 0; i < lines.size(); i++) {
    const cv::Rect bounding_box = lines[i].GetBoundingBox();
    cv::Point2f object_min = lines[i].GetPosition();
    cv::Point2f object_max =
        cv::Point2f{lines[i].GetPosition().x + bounding_box.width,
                    lines[i].GetPosition().y + bounding_box.height};

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

    for (std::ptrdiff_t y = body_min_y; y <= body_max_y; y++) {
      auto &row = m_Grid[y];
      if (row.empty()) {
        row.resize(m_Columns);
      }
      for (std::ptrdiff_t x = body_min_x; x <= body_max_x; x++) {
        row[x].push_back(lines[i].GetIndex());
      }
    }
  }
}

std::list<std::pair<int, int>> Grid::GetCollisionPairs() const {
  std::unordered_multimap<int, int> checked_pairs;
  std::list<std::pair<int, int>> pairs;

  for (auto row_it = m_Grid.begin(); row_it != m_Grid.end(); row_it++) {
    for (auto cell_it = row_it->begin(); cell_it != row_it->end(); ++cell_it) {
      // no possible intersection in this cell
      if (cell_it->size() <= 1) {
        continue;
      }
      for (auto lineA_it = cell_it->begin(); lineA_it != cell_it->end();
           ++lineA_it) {
        for (auto lineB_it = cell_it->begin(); lineB_it != lineA_it;
             ++lineB_it) {
          std::pair<int, int> pair;
          if (*lineA_it < *lineB_it) {
            pair = std::make_pair(*lineA_it, *lineB_it);
          } else {
            pair = std::make_pair(*lineB_it, *lineA_it);
          }
          if (!HasBeenChecked(checked_pairs, pair)) {
            pairs.push_back(pair);
            checked_pairs.insert(pair);
          }
        }
      }
    }
  }
  return pairs;
}

bool Grid::HasBeenChecked(std::unordered_multimap<int, int> &checked_pairs,
                          std::pair<int, int> pair) const {
  auto [first, second] = checked_pairs.equal_range(pair.first);
  for (auto &i = first; i != second; ++i) {
    if (i->second == pair.second) {
      return true;
    }
  }
  return false;
}

} // namespace mathboard
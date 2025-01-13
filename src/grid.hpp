#pragma once

// lib
// opencv
#include <opencv2/opencv.hpp>
// spdlog
#include <spdlog/spdlog.h>

// std
#include <concepts>
#include <fstream>
#include <list>
#include <vector>
#include <unordered_set>

template <typename T> concept HasPosition = requires(T object) {
  object.GetPosition();
};

template <typename T> concept HasDimensions = requires(T object) {
  object.GetWidth();
  object.GetHeight();
};

struct Position2f {
  float x;
  float y;
};
struct BoundingBox {
  int width;
  int height;
};

namespace mathboard {
// The Grid class handles broad-phase intersection detection by dividing space
// into cells, each containing potential object intersection. Grid as a class
// doesn't own pointers to objects which stores.
template <typename T> requires HasPosition<T> &&HasDimensions<T> class Grid {
public:
  // Constructor that sets up a grid covering a specified area,
  // defined by the top-left and bottom-right corners, with each cell having the
  // given size.
  Grid(const cv::Point2f &top_left_corner, const cv::Point2f &bot_right_corner,
       const cv::Size2i &cell_size)
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

  // Insert object to grid.
  void Insert(T *object) {
    const Position2f pos =
        Position2f{object->GetPosition().x, object->GetPosition().y};
    Position2f object_min = pos;
    const BoundingBox bounding_box =
        BoundingBox{object->GetWidth(), object->GetHeight()};
    Position2f object_max =
        Position2f{pos.x + bounding_box.width, pos.y + bounding_box.height};

    // calculating position of vertices in grid
    // decrese width and height because containers are 0 index based
    int body_min_x = static_cast<int>(
        std::floor((object_min.x - m_TopLeftCorner.x) / m_CellSize.width));
    body_min_x = std::clamp(body_min_x, 0, static_cast<int>(m_Columns - 1));

    int body_max_x = static_cast<int>(
        std::floor((object_max.x - m_TopLeftCorner.x) / m_CellSize.width));
    body_max_x = std::clamp(body_max_x, 0, static_cast<int>(m_Columns - 1));

    int body_min_y = static_cast<int>(
        std::floor((object_min.y - m_TopLeftCorner.y) / m_CellSize.height));
    body_min_y = std::clamp(body_min_y, 0, static_cast<int>(m_Rows - 1));

    int body_max_y = static_cast<int>(
        std::floor((object_max.y - m_TopLeftCorner.y)) / m_CellSize.height);
    body_max_y = std::clamp(body_max_y, 0, static_cast<int>(m_Rows - 1));

    for (std::ptrdiff_t x = body_min_x; x <= body_max_x; x++) {
      for (std::ptrdiff_t y = body_min_y; y <= body_max_y; y++) {
        m_Grid[x + m_Columns * y].push_back(object);
      }
    }
    m_Size++;
  }

  // Returns list of pairs of objects inside the same grid cell.
  std::list<std::pair<T *, T *>> GetIntersections() const {
    std::unordered_multimap<T *, T *> checked_pairs;
    std::list<std::pair<T *, T *>> pairs;

    for (std::size_t i = 0; i < m_Grid.size(); i++) {
      // no possible intersection in this cell
      if (m_Grid[i].size() <= 1) {
        continue;
      }
      for (T *element_A : m_Grid[i]) {
        for (T *element_B : m_Grid[i]) {
          if (element_A == element_B) {
            break;
          }
          std::pair<T *, T *> pair;
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

  // Clears all objects from grid. Without deleting grid structure.
  void Clear() {
    for (auto &cell : m_Grid) {
      cell.clear();
    }
    m_Size = 0;
  }

  // Returns number of unique objects inside Grid
  std::size_t Size() const {
    return m_Size;
  }

private:
  // Checks if a pair of objects has already been checked for intersection.
  bool HasBeenChecked(std::unordered_multimap<T *, T *> &checked_pairs,
                      std::pair<T *, T *> pair) const {
    auto [first, second] = checked_pairs.equal_range(pair.first);
    for (auto &i = first; i != second; ++i) {
      if (i->second == pair.second) {
        return true;
      }
    }
    return false;
  }

private:
  cv::Point2f m_TopLeftCorner{0.0f, 0.0f};
  cv::Point2f m_BotRightCorner{0.0f, 0.0f};
  cv::Size2i m_CellSize{0, 0};
  // Number of unique objects inside Grid
  std::size_t m_Size{0};
  std::uint32_t m_Rows{0};
  std::uint32_t m_Columns{0};
  std::vector<std::vector<T *>> m_Grid;
};
} // namespace mathboard
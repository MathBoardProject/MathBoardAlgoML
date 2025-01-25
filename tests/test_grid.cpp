#include <gtest/gtest.h>
#include <opencv2/core/types.hpp>
#define private public 
#include "../src/grid.hpp"
#undef private

namespace {

// simple struct for testing purpose
struct Shape {
  Shape(int index, cv::Point2f position, cv::Size2i size) :
    m_Index(index), m_Position(position), m_Size(size) {}
public:
  void SetPosition(float x, float y){
    m_Position.x = x;
    m_Position.y = y;
  }

  void SetSize(int width, int height) {
    m_Size.width = width;
    m_Size.height = height;
  }

  cv::Point2f GetPosition() const {return m_Position;}
  cv::Size2i GetSize() const { return m_Size; }
  int GetIndex() const {return m_Index;}
private:
  int m_Index{0};
  cv::Point2f m_Position;
  cv::Size2i m_Size;
};


class GridTest : public ::testing::Test {
protected:
  GridTest() :
    m_Grid(cv::Point2f(0.0f, 0.0f), cv::Point2f(100.0f, 100.0f), cv::Size2i(10, 10)) {}
  mathboard::Grid<Shape> GetGrid() const {return m_Grid;}
protected:
  mathboard::Grid<Shape> m_Grid;
};

TEST_F(GridTest, Constructor) {
  const cv::Point2f top_left_corner = cv::Point2f(0.0f, 0.0f);
  const cv::Point2f bot_right_corner = cv::Point2f(100.0f, 100.0f);
  const cv::Size2i cell_size = cv::Size2i(10, 10);
  mathboard::Grid<Shape> grid = GetGrid();
  EXPECT_EQ(top_left_corner, grid.m_TopLeftCorner);
  EXPECT_EQ(bot_right_corner, grid.m_BotRightCorner);
  EXPECT_EQ(cell_size, grid.m_CellSize);
}

TEST_F(GridTest, Insert) {
  Shape s0 = Shape(0, cv::Point2f(40, 0), cv::Size2i(10, 10));
  mathboard::Grid<Shape> grid = GetGrid();
  grid.Insert(&s0);
  // calculates shape position on grid
  const cv::Point2i top_left_corner_grid = 
    cv::Point2i(s0.GetPosition().x / grid.m_CellSize.width, s0.GetPosition().y / grid.m_CellSize.height);
  const cv::Point2i bot_right_corner_grid = cv::Point2i(
      (s0.GetPosition().x + s0.GetSize().width) / grid.m_CellSize.width,
      (s0.GetPosition().y + s0.GetSize().height) / grid.m_CellSize.height);
  EXPECT_FALSE(grid.m_Grid[top_left_corner_grid.x + grid.m_Columns * top_left_corner_grid.y].empty());
  EXPECT_FALSE(grid.m_Grid[top_left_corner_grid.x + grid.m_Columns * bot_right_corner_grid.y].empty());
  EXPECT_FALSE(grid.m_Grid[bot_right_corner_grid.x + grid.m_Columns * bot_right_corner_grid.y].empty());
  EXPECT_FALSE(grid.m_Grid[bot_right_corner_grid.x + grid.m_Columns * top_left_corner_grid.y].empty());
}

TEST_F(GridTest, Size) {
  Shape s0 = Shape(0, cv::Point2f(40, 40), cv::Size2i(10, 5));
  Shape s1 = Shape(1, cv::Point2f(4, 60), cv::Size2i(20, 20));
  Shape s2 = Shape(2, cv::Point2f(20, 90), cv::Size2i(6, 6));
  mathboard::Grid<Shape> grid = GetGrid();
  grid.Insert(&s0);
  grid.Insert(&s1);
  grid.Insert(&s2);
  EXPECT_EQ(grid.Size(), 3);
}

TEST_F(GridTest, Clear) {
  Shape s0 = Shape(0, cv::Point2f(40, 40), cv::Size2i(10, 5));
  Shape s1 = Shape(1, cv::Point2f(4, 60), cv::Size2i(20, 20));
  Shape s2 = Shape(2, cv::Point2f(20, 90), cv::Size2i(6, 6));
  mathboard::Grid<Shape> grid = GetGrid();
  grid.Insert(&s0);
  grid.Insert(&s1);
  grid.Insert(&s2);
  grid.Clear();
  EXPECT_EQ(grid.Size(), 0);
}

TEST_F(GridTest, HasBeenChecked) {
  Shape s0 = Shape(0, cv::Point2f(40, 40), cv::Size2i(10, 5));
  Shape s1 = Shape(1, cv::Point2f(4, 60), cv::Size2i(20, 20));
  Shape s2 = Shape(2, cv::Point2f(20, 90), cv::Size2i(6, 6));
  std::unordered_multimap<Shape *, Shape *> checked_pairs = {
    std::pair<Shape *, Shape *>(&s0, &s1)
  };
  std::pair<Shape *, Shape *> pair0 = std::make_pair(&s0, &s1);
  std::pair<Shape *, Shape *> pair1 = std::make_pair(&s1, &s2);
  mathboard::Grid grid = this->GetGrid();
  EXPECT_TRUE(grid.HasBeenChecked(checked_pairs, pair0));
  EXPECT_FALSE(grid.HasBeenChecked(checked_pairs, pair1));
}

TEST_F(GridTest, NoIntersection) {
  Shape s0 = Shape(0, cv::Point2f(40, 40), cv::Size2i(10, 5));
  Shape s1 = Shape(1, cv::Point2f(4, 60), cv::Size2i(20, 20));
  Shape s2 = Shape(2, cv::Point2f(20, 90), cv::Size2i(6, 6));
  mathboard::Grid<Shape> grid = GetGrid();
  grid.Insert(&s0);
  grid.Insert(&s1);
  grid.Insert(&s2);
  EXPECT_EQ(grid.GetIntersections().size(), 0);
  
}

TEST_F(GridTest, ExistingIntersection) {
  Shape s0 = Shape(0, cv::Point2f(21.37f, 69.0f), cv::Size2i(10, 5));
  Shape s1 = Shape(1, cv::Point2f(21.37f, 69.0f), cv::Size2i(20, 20));
  Shape s2 = Shape(2, cv::Point2f(21.37f, 69.0f), cv::Size2i(6, 6));
  mathboard::Grid<Shape> grid = GetGrid();
  grid.Insert(&s0);
  grid.Insert(&s1);
  grid.Insert(&s2);
  EXPECT_EQ(grid.GetIntersections().size(), 3);
  std::pair<Shape *, Shape *> pair0 = std::make_pair(&s0, &s1);
  std::pair<Shape *, Shape *> pair2 = std::make_pair(&s1, &s2);
  EXPECT_EQ(grid.GetIntersections().front(), pair0);
  EXPECT_EQ(grid.GetIntersections().back(), pair2);
}
}
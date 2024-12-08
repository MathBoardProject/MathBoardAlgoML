#pragma once

#include <vector>
#include <list>
#include <set>
#include <opencv2/core.hpp>
#include "opencv2/opencv.hpp"
#include "Grid.hpp"

// temporary struct for holding information about image of line
// some of this members may be useless
struct LineInfo
{
  // idx of image passed to main 
  int idx;
  // /position is aligned with img top left corner 
  cv::Point2f position;
  std::vector< std::vector< cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::Rect GetBB()
  {
    return cv::boundingRect(contours);
  }
};

namespace mathboard 
{
    /*
      The Grid class handles broad-phase collision detection by dividing space into cells, 
      each containing potential lines intersection.
    */
    class Grid 
    {
    public:
      Grid() = default;
      /*
        Constructor that sets up a grid covering a specified area, 
        defined by the top-left and bottom-right corners, with each cell having the given size.
      */
      Grid(const cv::Point2f &topLeftCorner, const cv::Point2f &botRightCorner, float cellSize);
    public:
      /*
        Inserts all lines into grid in appropriate place in space
      */
      void InsertObjects(std::vector<LineInfo> &lines);
      /*
        Updates the grid with the positions of the objects.
        It places each object's ID in the appropriate grid cell based on its position.
      */
      std::list< std::pair<int, int> > GetCollisionPairs() const;
    private:
      // Checks if a pair of objects has already been checked for collisions.
      bool HasBeenChecked(std::unordered_multimap<int, int> &checkedPairs, std::pair<int, int> pair) const;
    protected:
      cv::Point2f m_TopLeftCorner{0.0f, 0.0f};
      cv::Point2f m_BotRightCorner{100.0f, 100.0f};
      float m_CellSize{10.0f};
      int m_Width{10};
      int m_Height{10};
      std::vector< std::vector< std::vector<int> > > m_Grid;
    };
}
#include "Grid.hpp"
#include <fstream>
#include "common.hpp"

namespace mathboard 
{
  Grid::Grid(const cv::Point2f &topLeftCorner, const cv::Point2f &botRightCorner, float cellSize) :
      m_TopLeftCorner(topLeftCorner), m_BotRightCorner(botRightCorner),
      m_CellSize(cellSize),
      m_Height((m_BotRightCorner.y - m_TopLeftCorner.y) / m_CellSize),
      m_Width((m_BotRightCorner.x - m_TopLeftCorner.x) / m_CellSize) 
  {
    if(m_CellSize <= 0) 
    {
        std::ofstream debug_stream("debug_output.txt", std::ios::app);
        debug_stream << "Cell size have to be positive number\n";
        debug_stream.close();
    }
    m_Grid.resize(m_Height);
    for(int i = 0; i < m_Grid.size(); i++) 
    {
      m_Grid[i].resize(m_Width);
    }
  }

  void Grid::InsertObjects(std::vector<LineInfo> &lines)
  {
    for(int i = 0; i < lines.size(); i++)
    {
      const cv::Rect bb = lines[i].GetBB();
      cv::Point2f objectMin = lines[i].position;
      cv::Point2f objectMax = cv::Point2f{lines[i].position.x + bb.x, lines[i].position.y + bb.y};

      // calculating position of vertices in grid 
      // decrese width and height because containers are 0 index based
      int bodyMinX = static_cast<int>(std::floor((objectMin.x - m_TopLeftCorner.x) / m_CellSize));
      bodyMinX = std::clamp(bodyMinX, 0, (int)m_Width - 1);

      int bodyMaxX = static_cast<int>(std::floor((objectMax.x - m_TopLeftCorner.x) / m_CellSize));
      bodyMaxX = std::clamp(bodyMaxX, 0, (int)m_Width - 1);

      int bodyMinY = static_cast<int>(std::floor((objectMin.y - m_TopLeftCorner.y) / m_CellSize));
      bodyMinY = std::clamp(bodyMinY, 0, (int)m_Height - 1);

      int bodyMaxY = static_cast<int>(std::floor((objectMax.y - m_TopLeftCorner.y)) / m_CellSize);
      bodyMaxY = std::clamp(bodyMaxY, 0, (int)m_Height - 1);

      for(int y = bodyMinY; y <= bodyMaxY; y++)
      {
          auto &row = m_Grid[y];
          if(row.empty())
          {
              row.resize(m_Width);
          } 
          for(int x = bodyMinX; x <= bodyMaxX; x++)
          {
              row[x].push_back(lines[i].idx);
          }
      }
    }
  }

  std::list< std::pair<int, int> > Grid::GetCollisionPairs() const 
  {
    std::unordered_multimap<int, int> checkedPairs;
    std::list< std::pair<int, int> > pairs;
    
    for(auto rowIt = m_Grid.begin(); rowIt != m_Grid.end(); rowIt++) 
    {
      for(auto cellIt = rowIt->begin(); cellIt != rowIt->end(); ++cellIt) 
      {
        // no possible intersection in this cell 
        if(cellIt->size() <= 1) 
        {
          continue;
        }
        for(auto lineAIt = cellIt->begin(); lineAIt != cellIt->end(); ++lineAIt) 
        {
          for(auto lineBIt = cellIt->begin(); lineBIt != lineAIt; ++lineBIt) 
          {
            std::pair<int, int> pair;
            if(*lineAIt < *lineBIt) 
            {
              pair = std::make_pair(*lineAIt, *lineBIt);
            } 
            else 
            {
              pair = std::make_pair(*lineBIt, *lineAIt);
            }
            if(!HasBeenChecked(checkedPairs, pair))
            {
                pairs.push_back(pair);
                checkedPairs.insert(pair);
            }
          }
        }
      }
    }
    return pairs;
  }

  bool Grid::HasBeenChecked(std::unordered_multimap<int, int> &checkedPairs, std::pair<int, int> pair) const 
  {
    auto [ first, second ] = checkedPairs.equal_range(pair.first);
    for(auto &i = first; i != second; ++i)
    {
        if(i->second == pair.second)
        {
            return true;
        }
    }
    return false;
  }
}
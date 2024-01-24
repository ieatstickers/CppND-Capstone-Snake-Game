
#include "obstacle.h"

Obstacle::Obstacle(int x, int y, int size, int duration) : x(x), y(y), size(size), duration(duration) {
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      _occupiedCoordinates.push_back(std::make_pair(x + i, y + j));
    }
  }
}

std::vector<std::pair<int, int>> Obstacle::GetOccupiedCoordinates() const {
  return _occupiedCoordinates;
}

bool Obstacle::CoordinatesOverlap(int xCoord, int yCoord) const {
  for (std::pair<int, int> const &occupiedCoordinate : GetOccupiedCoordinates()) {
    if (occupiedCoordinate.first == xCoord && occupiedCoordinate.second == yCoord) return true;
  }
  
  return false;
}

bool Obstacle::OverlapsWith(Obstacle const &other) const {
  Obstacle smallest = size < other.size ? *this : other;
  Obstacle largest = size < other.size ? other : *this;
  
  for (std::pair<int, int> occupiedCoords : smallest.GetOccupiedCoordinates()) {
    if (largest.CoordinatesOverlap(occupiedCoords.first, occupiedCoords.second)) return true;
  }
  
  return false;
}
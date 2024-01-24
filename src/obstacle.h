
#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <vector>

class Obstacle {
public:
    const int x;
    const int y;
    const int size;
    const int duration;
    Obstacle(int x, int y, int size, int duration);
    bool CoordinatesOverlap(int x, int y) const;
    bool OverlapsWith(Obstacle const &other) const;
    std::vector<std::pair<int, int>> GetOccupiedCoordinates() const;

private:
    std::vector<std::pair<int, int>> _occupiedCoordinates;
  
};

#endif
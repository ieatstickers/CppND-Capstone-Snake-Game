#ifndef GAME_H
#define GAME_H

#include <random>
#include <mutex>
#include <atomic>
#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "snake.h"
#include "game_data.h"
#include "obstacle.h"

class Game {
 public:
  Game(std::size_t grid_width, std::size_t grid_height, std::shared_ptr<GameData> gameDataPtr);
  ~Game();
  void Run(Controller const &controller, Renderer &renderer,
           std::size_t target_frame_duration);
  int GetScore() const;
  int GetSize() const;

 private:
  void ManageObstacleLifetime(const std::shared_ptr<Obstacle>& obstaclePtr);
  int grid_width;
  int grid_height;
  Snake snake;
  SDL_Point food;
  std::shared_ptr<GameData> gameData;
  std::vector<std::shared_ptr<Obstacle>> obstacles;
  std::vector<std::shared_ptr<std::thread>> obstacle_threads;

  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<int> random_w;
  std::uniform_int_distribution<int> random_h;
  std::uniform_int_distribution<int> random_obstacle_size;
  std::uniform_int_distribution<int> random_obstacle_duration;
  std::condition_variable condition_variable;
  std::atomic<bool> terminateObstacleThreads;
  std::mutex mtx;

  int score{0};

  void PlaceFood();
  void PlaceObstacle();
  void Update();
};

#endif
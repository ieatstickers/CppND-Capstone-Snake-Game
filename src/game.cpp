#include <iostream>
#include <thread>
#include <mutex>
#include <algorithm>
#include "SDL.h"
#include "game.h"
#include "game_data.h"

Game::Game(std::size_t grid_width, std::size_t grid_height, std::shared_ptr<GameData> data)
    : grid_width(grid_width),
      grid_height(grid_height),
      snake(grid_width, grid_height),
      engine(dev()),
      gameData(std::move(data)),
      random_w(0, static_cast<int>(grid_width - 1)),
      random_h(0, static_cast<int>(grid_height - 1)),
      random_obstacle_size(2, 4),
      random_obstacle_duration(5000, 30000),
      terminateObstacleThreads(false) {
  PlaceFood();
}

Game::~Game() {
  terminateObstacleThreads = true;
  for (std::shared_ptr<std::thread> thread : obstacle_threads) {
    thread->join();
  }
}

void Game::Run(Controller const &controller, Renderer &renderer,
               std::size_t target_frame_duration) {
  Uint32 title_timestamp = SDL_GetTicks();
  Uint32 frame_start;
  Uint32 frame_end;
  Uint32 frame_duration;
  int frame_count = 0;
  bool running = true;

  while (running) {
    frame_start = SDL_GetTicks();
    
    // Lock mutex before handling input (which updates the snake)
    std::unique_lock<std::mutex> lock(mtx);

    // Input, Update, Render - the main game loop.
    controller.HandleInput(running, snake);
    
    // Unlock mutex before calling Update() (which locks the mutex itself)
    lock.unlock();
    
    Update();
    
    // Create obstacle points
    std::vector<SDL_Point> obstaclePoints;
    
    // Lock mutex before reading obstacles and rendering
    lock.lock();
    
    for (std::shared_ptr<Obstacle> &obstacle : obstacles) {
      for (std::pair<int, int> obstacleCoordinates : obstacle->GetOccupiedCoordinates()) {
        SDL_Point point;
        point.x = obstacleCoordinates.first;
        point.y = obstacleCoordinates.second;
        obstaclePoints.push_back(point);
      }
    }
    
    renderer.Render(snake, food, obstaclePoints);
    
    // Unlock now snake and obstacles no longer need to be read or modified
    lock.unlock();

    frame_end = SDL_GetTicks();

    // Keep track of how long each loop through the input/update/render cycle
    // takes.
    frame_count++;
    frame_duration = frame_end - frame_start;

    // After every second, update the window title.
    if (frame_end - title_timestamp >= 1000) {
      renderer.UpdateWindowTitle(score, frame_count, gameData->GetTopScore());
      frame_count = 0;
      title_timestamp = frame_end;
    }

    // If the time for this frame is too small (i.e. frame_duration is
    // smaller than the target ms_per_frame), delay the loop to
    // achieve the correct frame rate.
    if (frame_duration < target_frame_duration) {
      SDL_Delay(target_frame_duration - frame_duration);
    }
  }
}

void Game::PlaceFood() {
  int x, y;
  while (true) {
    x = random_w(engine);
    y = random_h(engine);
    
    // Lock mutex before reading from snake and obstacles
    std::unique_lock<std::mutex> lock(mtx);
    // If new food position overlaps with the snake, regenerate position
    if (snake.SnakeCell(x, y)) continue;
    
    bool overlapsObstacle = false;
    for (std::shared_ptr<Obstacle> &obstaclePtr : obstacles) {
      if (obstaclePtr->CoordinatesOverlap(x, y)) {
        overlapsObstacle = true;
        break;
      }
    }
    
    // Unlock now shared data is no longer needs to be read
    lock.unlock();
    
    // If new food position overlaps with an obstacle, regenerate position
    if (overlapsObstacle) continue;
    
    food.x = x;
    food.y = y;
    return;
  }
}

void Game::PlaceObstacle() {
  int x, y, size, duration;
  
  while (true) {
    x = random_w(engine);
    y = random_h(engine);
    size = random_obstacle_size(engine);
    
    
    // If obstacle won't fit on grid, generate a new one
    if (x + size > grid_width || y + size > grid_height) {
      continue;
    }
    
    duration = random_obstacle_duration(engine);
    
    // Create new obstacle
    Obstacle newObstacle(x, y, size, duration);
    
    // Lock mutex before reading/modifying from snake and obstacles
    std::lock_guard<std::mutex> lock(mtx);
    
    // Check to see if it overlaps with the snake
    bool overlapsWithSnake = false;
    for (std::pair<int, int> coords : newObstacle.GetOccupiedCoordinates()) {
      if (snake.SnakeCell(coords.first, coords.second)) {
        overlapsWithSnake = true;
        break;
      }
    }
    
    // If it overlaps with the snake, try generating a new obstacle
    if (overlapsWithSnake) {
      continue;
    }
    
    // Check to see if it overlaps with an existing obstacle
    bool overlapsExistingObstacle = false;
    for (std::shared_ptr<Obstacle> &existingObstacle : obstacles) {
      if (existingObstacle->OverlapsWith(newObstacle)) {
        overlapsExistingObstacle = true;
        break;
      }
    }
    
    // If it overlaps with existing obstacle, try generating a new obstacle
    if (overlapsExistingObstacle) {
      continue;
    }
    
    std::shared_ptr<Obstacle> obstaclePointer = std::make_shared<Obstacle>(newObstacle);
    obstacles.push_back(obstaclePointer);
    
    std::thread obstacleThread = std::thread(&Game::ManageObstacleLifetime, this, obstaclePointer);
    obstacle_threads.push_back(std::make_shared<std::thread>(std::move(obstacleThread)));
    
    return;
  }
}

void Game::ManageObstacleLifetime(std::shared_ptr<Obstacle> const& obstaclePtr) {
  auto end = std::chrono::system_clock::now() + std::chrono::milliseconds(obstaclePtr->duration);
  
  while(!terminateObstacleThreads) {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    
    // If obstacle duration has passed, break while loop
    auto newNow = std::chrono::system_clock::now();
    if (newNow > end) {
      lock.lock();
      // Remove the obstacle from the vector
      obstacles.erase(std::remove(obstacles.begin(), obstacles.end(), obstaclePtr), obstacles.end());
      break;
    }
    
    lock.lock();
    // If snake has collided with this object, set snake alive to false and return
    if (obstaclePtr->CoordinatesOverlap(static_cast<int>(snake.head_x), static_cast<int>(snake.head_y))) {
      snake.alive = false;
      return;
    }
    
    // Wait for 1 millisecond or until interrupted
    if(condition_variable.wait_for(lock, std::chrono::milliseconds(1)) == std::cv_status::no_timeout) {
      // Interrupted by main thread
      break;
    }
  }
}



void Game::Update() {
  std::unique_lock<std::mutex> lock(mtx);
  
  if (!snake.alive) return;
  
  snake.Update();

  int new_x = static_cast<int>(snake.head_x);
  int new_y = static_cast<int>(snake.head_y);
  
  lock.unlock();

  // Check if there's food over here
  if (food.x == new_x && food.y == new_y) {
    score++;
    PlaceFood();
    
    // If score divisible by 3, place an obstacle on the grid
    if (score % 3 == 0) PlaceObstacle();
    
    // Grow snake and increase speed.
    snake.GrowBody();
    snake.speed += 0.02;
    
    if (score > gameData->GetTopScore()) {
      gameData->SetTopScore(score);
    }
  }
}

int Game::GetScore() const { return score; }
int Game::GetSize() const { return snake.size; }
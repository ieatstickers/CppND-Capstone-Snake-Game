#include <iostream>
#include "controller.h"
#include "game.h"
#include "renderer.h"
#include "game_data.h"

int main() {
  constexpr std::size_t kFramesPerSecond{60};
  constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
  constexpr std::size_t kScreenWidth{640};
  constexpr std::size_t kScreenHeight{640};
  constexpr std::size_t kGridWidth{32};
  constexpr std::size_t kGridHeight{32};

  Renderer renderer(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight);
  Controller controller;
  std::shared_ptr<GameData> gameDataPtr = std::make_shared<GameData>();
  int topScoreBeforeGame = gameDataPtr->GetTopScore();
  Game game(kGridWidth, kGridHeight, gameDataPtr);
  game.Run(controller, renderer, kMsPerFrame);
  std::cout << "Game has terminated successfully!\n";
  std::cout << "Score: " << game.GetScore() << "\n";
  std::cout << "Size: " << game.GetSize() << "\n";
  if (gameDataPtr->GetTopScore() > topScoreBeforeGame) {
    std::cout << "Nice! You set a new top score!" << "\n";
  } else {
    std::cout << "Better luck next time! You didn't set a new top score this time." << "\n";
  }
  
  return 0;
}
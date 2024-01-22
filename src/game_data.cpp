
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "game_data.h"

// Constructor
GameData::GameData() : _topScore(0), _filePath("./game_data.txt"), _saveChangesOnUpdate(true), _savePending(false) {
  HydrateFromFile();
  _saveThread = std::thread(&GameData::SaveChangesOnUpdate, this);
}

GameData::~GameData() {
  _saveChangesOnUpdate = false;
  _saveCondition.notify_one();
  _saveThread.join();
}

// Public methods

int GameData::GetTopScore() const {
  return _topScore;
}

void GameData::SetTopScore(int topScore) {
  std::lock_guard<std::mutex> lock(_saveMutex);
  if (topScore < _topScore) {
    throw std::runtime_error("Cannot set a top score that is lower than the existing top score");
  }
  _topScore = topScore;
  _savePending = true;
  _saveCondition.notify_one();
}

// Private methods

void GameData::Save() {
  std::ofstream fileStream(_filePath);
  if (!fileStream.is_open()) throw std::runtime_error("Failed to save game data. Could not open file: " + _filePath);
  fileStream << "TOP_SCORE=" << _topScore << "\n";
  fileStream.close();
}

void GameData::SaveChangesOnUpdate() {
  while (true) {
    std::unique_lock<std::mutex> lock(_saveMutex);
    _saveCondition.wait(lock, [this]{ return _savePending || !_saveChangesOnUpdate; });
    
    if (!_saveChangesOnUpdate) break;
    
    Save();
    _savePending = false;
  }
}

void GameData::HydrateFromFile() {
  
  if (!std::filesystem::exists(_filePath)) return;
  
  std::ifstream fileStream(_filePath);
  if (!fileStream.is_open()) throw std::runtime_error("Failed to read game data. Could not open file: " + _filePath);
  std::string line, key, value;
  
  while(std::getline(fileStream, line)) {
    std::istringstream lineStream(line);
    
    if (std::getline(lineStream, key, '=') && std::getline(lineStream, value)) {
      if (key == "TOP_SCORE") _topScore = std::stoi(value);
    }
    
    // This can be extended to include other persisted data in future if needed
  }
  
  fileStream.close();
}
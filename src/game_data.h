
#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <string>
#include <thread>
#include <mutex>

class GameData {
  public:
    GameData();
    ~GameData();
    int GetTopScore() const;
    void SetTopScore(int topScore);
    GameData(const GameData&) = delete;
    GameData& operator=(const GameData&) = delete;
  private:
    void Save();
    void SaveChangesOnUpdate();
    void HydrateFromFile();
    int _topScore;
    std::string _filePath;
    std::thread _saveThread;
    std::mutex _saveMutex;
    std::condition_variable _saveCondition;
    bool _saveChangesOnUpdate;
    bool _savePending;
};

#endif
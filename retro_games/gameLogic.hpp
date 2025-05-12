#ifndef GAME_LOGIC_HPP
#define GAME_LOGIC_HPP

#include <algorithm>
#include <cstdio>
#include <mutex>
#include <vector>

#include "../controller/common.hpp"

namespace s21 {

#define DB_FILE "game_data.db"

class GameLogic {
 public:
  virtual ~GameLogic() = default;
  virtual void userInput(UserAction_t action, bool hold) = 0;
  virtual GameInfo_t updateCurrentState() = 0;
  virtual void gameTick() = 0;
  GameStatus getCurrentGameStatus() const { return currentGameStatus; }
  std::chrono::high_resolution_clock::time_point lastTickTime;

  static void saveHighScore(int highScore, int idGame) {
    struct Record {
      int32_t id;
      int32_t score;
    };

    std::vector<Record> records;
    FILE* file = fopen(DB_FILE, "rb");
    if (file) {
      while (true) {
        Record rec;
        if (fread(&rec, sizeof(rec), 1, file) != 1) break;
        records.push_back(rec);
      }
      fclose(file);
    }

    auto it =
        std::find_if(records.begin(), records.end(),
                     [idGame](const Record& r) { return r.id == idGame; });
    if (it != records.end()) {
      it->score = highScore;
    } else {
      records.push_back({idGame, highScore});
    }

    file = fopen(DB_FILE, "wb");
    if (file) {
      fwrite(records.data(), sizeof(Record), records.size(), file);
      fclose(file);
    }
  }

  static int loadHighScore(int idGame) {
    struct Record {
      int32_t id;
      int32_t score;
    };
    int score = 0;

    FILE* file = fopen(DB_FILE, "rb");
    if (file) {
      while (true) {
        Record rec;
        if (fread(&rec, sizeof(rec), 1, file) != 1) break;
        if (rec.id == idGame) {
          score = rec.score;
          break;
        }
      }
      fclose(file);
    }

    return score;
  }

 protected:
  GameInfo_t gameInfo;
  GameStatus currentGameStatus = GameStatus::INIT;
  std::mutex gameTickMutex;
};
}  // namespace s21

#endif  // GAME_LOGIC_HPP
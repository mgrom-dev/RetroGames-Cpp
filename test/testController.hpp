#ifndef TEST_CONTROLLER_HPP
#define TEST_CONTROLLER_HPP

#include <gtest/gtest.h>

#include "../controller/gameController.hpp"
#include "../gui/gameView.hpp"

namespace s21 {

class MockGameView : public GameView {
 public:
  void render(const GameInfo_t& gameInfo, GameStatus gameStatus,
              GameType gameType) override {
    if (gameInfo.field != nullptr &&
        (gameStatus != GameStatus::WIN && gameType != GameType::NONE))
      return;
  }

  GameType selectGame() override { return currentGameType; }

  void setCurrentGameType(GameType gameType) { currentGameType = gameType; }

 private:
  GameType currentGameType;
};

class GameControllerTest : public ::testing::Test, public GameController {
 public:
  GameControllerTest() {
    std::unique_ptr<s21::MockGameView> mv = std::make_unique<MockGameView>();
    mockView = mv.get();
    setView(std::move(mv));
  }

 protected:
  MockGameView* mockView = nullptr;
};

}  // namespace s21

#endif
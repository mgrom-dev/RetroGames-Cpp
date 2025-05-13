#ifndef GAME_CONTROLLER_HPP
#define GAME_CONTROLLER_HPP

#include <thread>

#include "../gui/gameView.hpp"
#include "../retro_games/snake/snakeLogic.hpp"
#include "../retro_games/tetris/tetrisLogic.hpp"
#include "common.hpp"

namespace s21 {
class GameController {
 public:
  GameController() = default;
  GameController(std::unique_ptr<GameView> view);

  void setView(std::unique_ptr<GameView> view);

  void userInput(Key key, bool hold);

  void run();

  void closeGame();

 protected:
  GameType gameType = GameType::NONE;
  std::unique_ptr<GameLogic> model;
  std::unique_ptr<GameView> view;
};
}  // namespace s21

#endif  // GAME_CONTROLLER_HPP
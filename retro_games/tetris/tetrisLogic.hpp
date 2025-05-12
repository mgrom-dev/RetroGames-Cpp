#ifndef TETRIS_LOGIC_HPP
#define TETRIS_LOGIC_HPP

#include "../gameLogic.hpp"

namespace s21 {
#define NEXT_WIDTH 4
#define NEXT_HEIGHT 4

class TetrisLogic : public GameLogic {
 public:
  TetrisLogic();
  ~TetrisLogic();
  void userInput(UserAction_t action, bool hold) override;
  GameInfo_t updateCurrentState() override;
  void gameTick() override;
};
}  // namespace s21

#endif  // TETRIS_LOGIC_HPP
#ifndef SNAKE_LOGIC_HPP
#define SNAKE_LOGIC_HPP

#include "../gameLogic.hpp"

namespace s21 {

class SnakeLogic : public GameLogic {
 public:
  enum class Field { EMPTY, FOOD, HEAD_LEFT, HEAD_RIGHT, HEAD_UP, HEAD_DOWN };
  enum class Direct { LEFT, RIGHT, UP, DOWN, NONE };

  SnakeLogic();
  ~SnakeLogic();
  void userInput(UserAction_t action, bool hold) override;
  GameInfo_t updateCurrentState() override;
  void gameTick() override;
};
}  // namespace s21

#endif  // SNAKE_LOGIC_HPP
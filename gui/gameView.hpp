#ifndef GAME_VIEW_HPP
#define GAME_VIEW_HPP

#include "../controller/common.hpp"

namespace s21 {
class GameController;

enum class Key { SPACE, ESC, LEFT, RIGHT, DOWN, UP, ENTER, P };

struct InputEvent {
  Key key;
  bool hold;
  bool noKey;
};

class GameView {
 public:
  virtual ~GameView() = default;
  virtual void render(const GameInfo_t& gameInfo, GameStatus gameStatus,
                      GameType gameType) = 0;
  virtual GameType selectGame() = 0;
};
}  // namespace s21

#endif  // GAME_VIEW_HPP
#ifndef COMMON_HPP
#define COMMON_HPP

namespace s21 {
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define NEXT_WIDTH 4
#define NEXT_HEIGHT 4

enum class GameStatus { INIT, INSTRUCTION, GAME, PAUSE, GAME_OVER, WIN };
enum class GameType { NONE, TETRIS, SNAKE };
enum class UserAction_t {
  Start,
  Pause,
  Terminate,
  Left,
  Right,
  Up,
  Down,
  Action
};

struct GameInfo_t {
  int** field;
  int** next;
  int score;
  int high_score;
  int level;
  int speed;
  int pause;

  GameInfo_t();
  GameInfo_t(const GameInfo_t& rhs);
  GameInfo_t(GameInfo_t&& rhs) noexcept;
  ~GameInfo_t();
  GameInfo_t& operator=(const GameInfo_t& rhs);
  bool operator!=(const GameInfo_t& rhs) const;
  bool operator==(const GameInfo_t& rhs) const;
};
}  // namespace s21

#endif  // COMMON_HPP
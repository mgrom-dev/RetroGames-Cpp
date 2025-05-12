#ifndef CONSOLE_VIEW_HPP
#define CONSOLE_VIEW_HPP

// turn on function clock_gettime in POSIX.1-2008
#define _XOPEN_SOURCE 700

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/select.h>  // alpine
#include <termios.h>
#include <unistd.h>
#endif

#include <iostream>
#include <thread>

#include "../gameView.hpp"

namespace s21 {
enum class Color {
  BLACK,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  PURPLE,
  LIGHT_BLUE,
  WHITE,
  DEFAULT
};
enum class Menu { START, PAUSE, INSTRUCTION, SELECT_GAME, NONE };

class GameController;

class ConsoleView : public GameView {
 public:
  ConsoleView();
  ~ConsoleView();
  void render(const GameInfo_t& gameInfo, GameStatus gameStatus,
              GameType gameType) override;
  void onInput(GameController& controller);
  InputEvent readKey();
  GameType selectGame() override;
  void startInputThread(GameController& controller);

 private:
  std::atomic<bool> running = false;
  std::atomic<bool> inSelectGame = false;
  std::thread keyReader;
  std::mutex renderMutex;
  Menu currentMenu = Menu::NONE;

#ifdef _WIN32
  DWORD oldMode;
#else
  struct termios oldt, newt;
#endif
};
}  // namespace s21

#endif  // CONSOLE_VIEW_HPP
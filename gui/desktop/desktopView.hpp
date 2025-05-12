#ifndef DESKTOP_VIEW_HPP
#define DESKTOP_VIEW_HPP

#include "../gameView.hpp"
#include "gameSelector.hpp"
#include "gameWindow.hpp"

namespace s21 {
enum class Menu { START, PAUSE, INSTRUCTION, SELECT_GAME, NONE };

class DesktopView : public GameView {
 public:
  DesktopView();
  ~DesktopView();
  void render(const GameInfo_t& gameInfo, GameStatus gameStatus,
              GameType gameType) override;
  GameType selectGame() override;
  void keyPressEvent(Key key);
  void setGameController(GameController& controller);
  void gameWindowClosed();

 private:
  GameWindow* gameWindow;
  GameController* gameController;
  std::mutex keyPressMutex;
  std::mutex renderMutex;
  Menu currentMenu = Menu::NONE;
};
}  // namespace s21

#endif  // DESKTOP_VIEW_HPP
#include "desktopView.hpp"

#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

#include "../../controller/gameController.hpp"

using namespace s21;

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);

  auto view = std::make_unique<DesktopView>();
  DesktopView* desktopViewPtr = view.get();

  GameController controller(std::move(view));
  desktopViewPtr->setGameController(controller);

  std::thread controllerThread([&controller]() { controller.run(); });

  int ret = app.exec();

  if (controllerThread.joinable()) {
    controllerThread.join();
  }

  return ret;
}

DesktopView::DesktopView() : gameWindow(nullptr) {
  gameWindow = new GameWindow(this);
}

DesktopView::~DesktopView() { delete gameWindow; }

void DesktopView::setGameController(GameController& controller) {
  gameController = &controller;
}

void DesktopView::gameWindowClosed() { gameController->closeGame(); }

static void showStartMenu(GameWindow* gameWindow) {
  const char* msg =
      "Press 'Enter'\n"
      "to start game\n\n"
      "Press 'up' key\n"
      "for instruction\n\n"
      "Press 'ESC'\n"
      "for exit";
  gameWindow->showInfoMessage(msg);
}

static void showInstructionsSnake(GameWindow* gameWindow) {
  const char* msg =
      "Snake Instructions:\n"
      "Control the snake\n"
      "Eat food to grow\n"
      "and score points\n"
      "Avoid walls and body\n"
      "to survive longer\n"
      "Level increases\n"
      "as you score more\n"
      "The game gets faster\n"
      "as levels go up\n\n"
      "Press 'Enter'\n"
      "to start game";
  static int field[FIELD_HEIGHT][FIELD_WIDTH] = {0};
  field[16][1] = 9;
  field[16][2] = 8;
  field[16][3] = 7;
  field[16][4] = 6;
  field[16][5] = 3;
  field[16][8] = 1;
  int* rows[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    rows[i] = field[i];
  }
  gameWindow->showInfoMessage(msg);
  gameWindow->setGameField(rows);
}

static void showInstructionsTetris(GameWindow* gameWindow) {
  const char* msg =
      "Tetris Instructions:\n"
      "Clear lines by\n"
      "filling with blocks\n"
      "Points:\n"
      "1 line=100, 2=300,\n"
      "3=750, 4 lines=1500\n"
      "Level increases\n"
      "every 600 points,\n"
      "max level is 10.\n"
      "The game gets faster\n"
      "as levels go up.\n"
      "Press Enter\n"
      "to start game";
  static int field[FIELD_HEIGHT][FIELD_WIDTH] = {0};
  field[19][0] = 1;
  field[19][1] = 1;
  field[19][2] = 1;
  field[19][3] = 1;
  field[19][7] = 1;
  field[19][8] = 1;
  field[19][9] = 1;
  field[17][4] = 1;
  field[17][5] = 1;
  field[17][6] = 1;
  field[16][5] = 1;
  int* rows[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    rows[i] = field[i];
  }
  gameWindow->showInfoMessage(msg);
  gameWindow->setGameField(rows);
}

static void showPauseMenu(GameWindow* gameWindow) {
  const char* msg =
      "Game paused\n\n"
      "press Enter\n"
      "to continue";
  gameWindow->showInfoMessage(msg);
}

static void showGameOverMenu(GameWindow* gameWindow) {
  const char* msg =
      "Game   over\n\n"
      "press Enter\n"
      "for restart";
  gameWindow->showInfoMessage(msg);
}

static void showWinMenu(GameWindow* gameWindow) {
  const char* msg =
      "Congrats, you win!\n\n"
      "Thanks for playing\n"
      "press  Enter\n"
      "to try again\n"
      "@by mgrom-dev";
  gameWindow->showInfoMessage(msg);
}

static void renderGame(GameWindow* gameWindow, const GameInfo_t* gameInfo,
                       GameType gameType) {
  gameWindow->hideInfoMessage();
  gameWindow->setScore(gameInfo->score);
  gameWindow->setLevel(gameInfo->level);
  gameWindow->setSpeed(gameInfo->speed);
  gameWindow->setHighScore(gameInfo->high_score);
  gameWindow->setGameField(gameInfo->field);
  if (gameType == GameType::TETRIS) {
    gameWindow->setNextField(gameInfo->next);
  }
}

void DesktopView::render(const GameInfo_t& gameInfo, GameStatus gameStatus,
                         GameType gameType) {
  static GameInfo_t oldGameInfo = {};
  static GameStatus oldGameStatus = GameStatus::PAUSE;
  static GameType oldGameType = GameType::SNAKE;
  static Menu oldMenu = Menu::NONE;
  std::lock_guard<std::mutex> lock(renderMutex);
  if (!gameWindow || (gameInfo == oldGameInfo && gameStatus == oldGameStatus &&
                      gameType == oldGameType && oldMenu == currentMenu)) {
    return;
  }

  if (gameStatus == GameStatus::INIT && currentMenu != Menu::START) {
    currentMenu = Menu::START;
    showStartMenu(gameWindow);
  } else if (gameStatus == GameStatus::INSTRUCTION) {
    currentMenu = Menu::INSTRUCTION;
    if (gameType == GameType::TETRIS) {
      showInstructionsTetris(gameWindow);
    } else if (gameType == GameType::SNAKE) {
      showInstructionsSnake(gameWindow);
    }
  } else if (gameStatus == GameStatus::PAUSE) {
    currentMenu = Menu::PAUSE;
    showPauseMenu(gameWindow);
  } else if (gameStatus == GameStatus::GAME_OVER) {
    currentMenu = Menu::NONE;
    showGameOverMenu(gameWindow);
  } else if (gameStatus == GameStatus::WIN) {
    currentMenu = Menu::NONE;
    showWinMenu(gameWindow);
  } else if (gameStatus == GameStatus::GAME) {
    currentMenu = Menu::NONE;
    renderGame(gameWindow, &gameInfo, gameType);
  }

  oldMenu = currentMenu;
  oldGameInfo.~GameInfo_t();
  oldGameInfo = gameInfo;
  oldGameStatus = gameStatus;
  oldGameType = gameType;
}

static bool keyIsHold(unsigned int key) {
  static unsigned int lastKey = 0;
  static struct timespec lastTime = {0, 0};
  struct timespec currentTime;
  bool hold = false;

  clock_gettime(CLOCK_MONOTONIC, &currentTime);  // get current time

  // Checks key has been pressed in a short period of time
  if (key == lastKey) {
    long timeDiff = (currentTime.tv_sec - lastTime.tv_sec) * 1000 +
                    (currentTime.tv_nsec - lastTime.tv_nsec) / 1000000;
    if (timeDiff < 80) {  // If interval is less 80 ms, than key is hold
      hold = true;
    }
  }

  // save current key and time for next iteration
  lastKey = key;
  lastTime = currentTime;

  return hold;
}

void DesktopView::keyPressEvent(Key key) {
  std::lock_guard<std::mutex> lock(keyPressMutex);
  gameController->userInput(key, keyIsHold(static_cast<unsigned int>(key)));
}

static void initSnakeGame(GameWindow* gameWindow) {
  QVector<QColor> colors = {
      QColor(0, 0, 0),    // background
      QColor(255, 0, 0),  // red
      QColor(0, 205, 0),  // green
      QColor(0, 205, 0),  // green
      QColor(0, 205, 0),  // green
      QColor(0, 205, 0),  // green
      QColor(0, 0, 238)   // blue
  };
  gameWindow->hideNextField();
  gameWindow->setTitle("SNAKE");
  gameWindow->setColors(colors);

  static int field[FIELD_HEIGHT][FIELD_WIDTH] = {0};
  int* rows[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    rows[i] = field[i];
  }
  gameWindow->setGameField(rows);

  gameWindow->setVisiblity(true);
}

static void initTetrisGame(GameWindow* gameWindow) {
  QVector<QColor> colors = {
      QColor(0, 0, 0),   // background
      QColor(0, 0, 238)  // blue
  };
  gameWindow->setTitle("TETRIS");
  gameWindow->setColors(colors);

  QVector<QColor> colorsNext = {
      QColor(0, 0, 0, 0),  // background
      QColor(0, 0, 238)    // blue
  };
  gameWindow->setColorsNext(colorsNext);

  static int field[FIELD_HEIGHT][FIELD_WIDTH] = {0};
  int* rows[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    rows[i] = field[i];
  }
  gameWindow->setGameField(rows);

  static int fieldNext[NEXT_HEIGHT][NEXT_WIDTH] = {0};
  int* rowsNext[NEXT_HEIGHT];
  for (int i = 0; i < NEXT_HEIGHT; ++i) {
    rowsNext[i] = fieldNext[i];
  }
  gameWindow->setNextField(rowsNext);

  gameWindow->setVisiblity(true);
}

GameType DesktopView::selectGame() {
  if (gameWindow && gameWindow->isVisible()) {
    gameWindow->setVisiblity(false);
  }

  GameType gameType = GameType::NONE;
  GameSelector* gameSelect = new GameSelector();
  int result = gameSelect->exec();

  if (result == QDialog::Accepted) {
    gameType = gameSelect->getGameType();
    if (gameType == GameType::TETRIS) {
      initTetrisGame(gameWindow);
    } else if (gameType == GameType::SNAKE) {
      initSnakeGame(gameWindow);
    }
  }

  delete gameSelect;

  return gameType;
}
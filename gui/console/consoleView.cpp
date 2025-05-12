#include "consoleView.hpp"

#include "../../controller/gameController.hpp"

using namespace s21;

int main() {
  auto view = std::make_unique<ConsoleView>();
  ConsoleView* consoleViewPtr = view.get();
  GameController controller(std::move(view));

  consoleViewPtr->startInputThread(controller);
  controller.run();

  return 0;
}

static std::string& getOutputBuffer() {
  static std::string outputBuffer;
  static bool isInit = false;
  if (!isInit) {
    isInit = true;
    outputBuffer.reserve(4096);  // reserve memory for string
  }
  return outputBuffer;
}

static void flushOutput() {
  std::string& buffer = getOutputBuffer();
  std::cout << buffer << std::flush;
  buffer.clear();
}

static void moveAtXY(int x, int y) {
  std::string sx = std::to_string(x + 1);
  std::string sy = std::to_string(y + 1);
  getOutputBuffer() += "\x1B[" + sy + ";" + sx + "H";
}

static void printAtXY(int x, int y, const std::string& text) {
  moveAtXY(x, y);
  getOutputBuffer() += text;
}

static void printVLine(int x, int y, char symbol, int size) {
  for (int i = 0; i < size; i++) {
    moveAtXY(x, y + i);
    getOutputBuffer() += symbol;
  }
}

static void printHLine(int x, int y, char symbol, int size) {
  std::string line(size, symbol);
  moveAtXY(x, y);
  getOutputBuffer() += line;
}

static void clearGameArea(int startX, int startY, int endX, int endY) {
  std::string output;
  std::string spaces(endX - startX, ' ');
  std::string sy, sx;

  for (int y = startY; y <= endY; y++) {
    moveAtXY(startX, y);
    getOutputBuffer() += spaces;
  }
}

// move cursor to start point
static void moveToStart() { moveAtXY(FIELD_WIDTH * 2 + 3, 0); }

#ifdef _WIN32
// get compatible codes with linux
static unsigned int getLinuxCode(WCHAR codeUChar, WORD virtualCode) {
  unsigned int code = 0;

  if (codeUChar > 0) {
    switch (codeUChar) {
      case ' ':
      case 27:
        code = codeUChar;
        break;
      case 13:
        code = 10;
        break;
      case 112:
      case 80:
      case 65447:
      case 65415:
        code = 'p';
        break;
    }
  } else {
    switch (virtualCode) {
      case 37:
        code = 1792836;
        break;
      case 39:
        code = 1792835;
        break;
      case 38:
        code = 1792833;
        break;
      case 40:
        code = 1792834;
        break;
    }
  }

  return code;
}

static unsigned int readUTF8() {
  const int timeoutMs = 1000;
  DWORD count = 0;
  HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
  INPUT_RECORD input;
  DWORD readCount = 0;
  unsigned int code = 0;

  int duration = 0;
  auto startTime = std::chrono::high_resolution_clock::now();
  while (duration < timeoutMs) {
    if (GetNumberOfConsoleInputEvents(hStdIn, &count)) {
      while (count > 0) {
        ReadConsoleInput(hStdIn, &input, 1, &readCount);
        count--;

        if (!input.Event.KeyEvent.bKeyDown) {
          continue;
        }

        auto codeUChar = input.Event.KeyEvent.uChar.UnicodeChar;
        auto virtualCode = input.Event.KeyEvent.wVirtualKeyCode;
        code = getLinuxCode(codeUChar, virtualCode);
        if (code > 0) {
          break;
        }
      }
    }
    if (code > 0) {
      break;
    }
    auto now = std::chrono::high_resolution_clock::now();
    duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime)
            .count();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));  // repeat 10 ms
  }

  return code;
}
#else
static unsigned int readUTF8() {
  unsigned int code = 0;
  unsigned char bytes[4];

  fd_set set;
  FD_ZERO(&set);
  FD_SET(STDIN_FILENO, &set);

  struct timeval timeout;
  timeout.tv_sec = 1;  // timeout in sec
  timeout.tv_usec = 0;

  // timeout for input
  if (select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout) > 0) {
    ssize_t count = read(STDIN_FILENO, &bytes, 4);
    for (int i = 0; i < count; i++) {
      code = (code << 8) | bytes[i];
    }
  }

  return code;
}
#endif

static void clear() { getOutputBuffer() += "\x1B[2J\x1B[H"; }

static void setColor(Color text, Color background) {
  struct TextColor {
    Color color;
    int codeText;
    int codeBackground;
  };
  static const TextColor textColors[] = {
      {Color::BLACK, 30, 40},      {Color::RED, 31, 41},
      {Color::GREEN, 32, 42},      {Color::YELLOW, 33, 43},
      {Color::BLUE, 34, 44},       {Color::PURPLE, 35, 45},
      {Color::LIGHT_BLUE, 36, 46}, {Color::WHITE, 37, 47},
      {Color::DEFAULT, 38, 48}};
  const int numTextColors = sizeof(textColors) / sizeof(textColors[0]);

  int textColor = 38;
  int backgroundColor = 48;
  for (int i = 0; i < numTextColors; i++) {
    if (text == textColors[i].color) {
      textColor = textColors[i].codeText;
    }
    if (background == textColors[i].color) {
      backgroundColor = textColors[i].codeBackground;
    }
  }

  std::string sc = std::to_string(textColor);
  std::string sbc = std::to_string(backgroundColor);
  getOutputBuffer() += "\x1B[" + sc + ";" + sbc + "m";
}

static void setDefaultColor() { getOutputBuffer() += "\x1B[0m"; }

static void drawFieldBorders(GameType gameType) {
  // up border
  if (gameType == GameType::TETRIS) {
    printAtXY(0, 0, "--------TETRIS--------");
  } else if (gameType == GameType::SNAKE) {
    printAtXY(0, 0, "--------SNAKE---------");
  }

  // left and right boder
  printVLine(0, 1, '|', FIELD_HEIGHT);
  printVLine(FIELD_WIDTH * 2 + 1, 1, '|', FIELD_HEIGHT);

  // down border
  printHLine(0, FIELD_HEIGHT + 1, '-', FIELD_WIDTH * 2 + 2);
}

static void drawScoreBorders() {
  const int startX = FIELD_WIDTH * 2 + 3;
  const int startY = 8;
  const int width = 18;
  const int height = 4;

  // upper border
  printAtXY(startX, startY, "-------SCORE--------");

  // left and right border
  printVLine(startX, startY + 1, '|', height);
  printVLine(startX + width + 1, startY + 1, '|', height);

  // down border
  printHLine(startX, startY + height + 1, '-', width + 2);
}

static void drawNextShapeBorders() {
  const int startX = FIELD_WIDTH * 2 + 6;
  const int startY = 0;
  const int width = 6 * 2;
  const int height = 6;

  // upper border
  printAtXY(startX, startY, "-----NEXT-----");

  // left and right border
  printVLine(startX, startY + 1, '|', height);
  printVLine(startX + width + 1, startY + 1, '|', height);

  // down border
  printHLine(startX, startY + height + 1, '-', width + 2);

  printAtXY(FIELD_WIDTH * 2 + 3, 0, ">");
}

static void showInstructionsTetris() {
  clearGameArea(1, 1, FIELD_WIDTH * 2 + 1, FIELD_HEIGHT);

  setColor(Color::WHITE, Color::BLUE);
  printHLine(1, FIELD_HEIGHT, ' ', 6);
  printHLine(9, FIELD_HEIGHT, ' ', 12);
  printHLine(11, FIELD_HEIGHT - 1, ' ', 6);
  printVLine(19, FIELD_HEIGHT - 4, ' ', 4);
  printVLine(20, FIELD_HEIGHT - 4, ' ', 4);
  printHLine(5, FIELD_HEIGHT - 3, ' ', 6);
  printHLine(7, FIELD_HEIGHT - 2, ' ', 2);
  setDefaultColor();

  printAtXY(1, 1, "Tetris Instructions:");
  printAtXY(1, 2, "Clear    lines   by");
  printAtXY(1, 3, "filling with blocks");
  printAtXY(1, 4, "Points:");
  printAtXY(1, 5, "1 line=100,   2=300,");
  printAtXY(1, 6, "3=750,  4 lines=1500");
  printAtXY(1, 8, "Level      increases");
  printAtXY(1, 9, "every  600   points,");
  printAtXY(1, 10, "max   level  is  10.");
  printAtXY(1, 11, "The game gets faster");
  printAtXY(1, 12, "as   levels  go  up.");
  printAtXY(4, 14, "Press   Enter");
  printAtXY(4, 15, "to start game");
  moveToStart();

  flushOutput();
}

static void showInstructionsSnake() {
  clearGameArea(1, 1, FIELD_WIDTH * 2 + 1, FIELD_HEIGHT);

  setColor(Color::WHITE, Color::GREEN);
  printHLine(10, FIELD_HEIGHT - 2, ' ', 2);
  setColor(Color::WHITE, Color::LIGHT_BLUE);
  printHLine(4, FIELD_HEIGHT - 2, ' ', 6);
  setColor(Color::WHITE, Color::RED);
  printHLine(16, FIELD_HEIGHT - 2, ' ', 2);
  setDefaultColor();

  printAtXY(1, 1, "Snake Instructions:");
  printAtXY(2, 2, "Control the snake");
  printAtXY(3, 3, "using arrow keys");
  printAtXY(3, 4, "Eat food to grow");
  printAtXY(3, 5, "and score points");
  printAtXY(1, 6, "Avoid walls and body");
  printAtXY(3, 7, "to survive longer");
  printAtXY(3, 9, "Level increases");
  printAtXY(2, 10, "as you score more");
  printAtXY(1, 11, "The game gets faster");
  printAtXY(3, 12, "as levels go up");
  printAtXY(4, 14, "Press   Enter");
  printAtXY(4, 15, "to start game");
  moveToStart();

  flushOutput();
}

static void startMenu(GameType gameType) {
  const int posX = FIELD_WIDTH - 6;
  const int posY = FIELD_HEIGHT / 2 - 4;

  clear();
  drawFieldBorders(gameType);
  drawScoreBorders();
  if (gameType == GameType::TETRIS) {
    drawNextShapeBorders();
  }
  clearGameArea(1, 1, FIELD_WIDTH * 2 + 1, FIELD_HEIGHT);

  printAtXY(posX, posY, "Press   Enter");
  printAtXY(posX, posY + 1, "to start game");
  printAtXY(posX - 1, posY + 3, "Press  'up' key");
  printAtXY(posX - 1, posY + 4, "for instruction");
  setColor(Color::LIGHT_BLUE, Color::BLACK);
  printAtXY(posX + 2, posY + 6, "Press ESC");
  printAtXY(posX + 2, posY + 7, "for  exit");
  setDefaultColor();
  moveToStart();

  flushOutput();
}

static void pauseGame() {
  int posX = FIELD_WIDTH - 5;
  int posY = FIELD_HEIGHT / 2 - 1;

  printAtXY(posX, posY++, "Game paused");
  printAtXY(posX, posY++, "press Enter");
  printAtXY(posX, posY++, "to continue");
  moveToStart();

  flushOutput();
}

static void drawGameOver() {
  int posX = FIELD_WIDTH - 5;
  int posY = FIELD_HEIGHT / 2 - 1;
  printAtXY(posX, posY++, "Game   over");
  printAtXY(posX, posY++, "press Enter");
  printAtXY(posX, posY++, "for restart");
  moveToStart();
  flushOutput();
}

static void showCredits() {
  int posY = 5;
  printAtXY(2, posY, "Congrats, you win!");
  printAtXY(2, posY + 1, "Thanks for playing");
  printAtXY(5, posY + 3, "press  Enter");
  printAtXY(5, posY + 4, "to try again");
  printAtXY(5, posY + 6, "@by dylangog");
  moveToStart();
  flushOutput();
}

static void drawScore(const GameInfo_t* gameInfo) {
  int posY = 9;
  int posX = FIELD_WIDTH * 2 + 5;
  clearGameArea(posX, posY, posX + 16, posY + 3);
  printAtXY(posX, posY++, "Score: " + std::to_string(gameInfo->score));
  printAtXY(posX, posY++, "Level: " + std::to_string(gameInfo->level));
  printAtXY(posX, posY++, "Speed: " + std::to_string(gameInfo->speed));
  printAtXY(posX, posY++,
            "High Score: " + std::to_string(gameInfo->high_score));
}

static void drawNextShape(const GameInfo_t* gameInfo) {
  int posX = FIELD_WIDTH * 2 + 9;
  int posY = 2;
  int** nextShape = gameInfo->next;
  clearGameArea(posX, posY, posX + 8, posY + 4);

  for (int y = 0; y < NEXT_HEIGHT; y++) {
    for (int x = 0; x < NEXT_WIDTH; x++) {
      if (nextShape[y][x] != 0) {
        setColor(Color::WHITE, Color::BLUE);
        printAtXY(posX + 2 * x, posY + y, " ");
        printAtXY(posX + 2 * x + 1, posY + y, " ");
        setDefaultColor();
      }
    }
  }
}

static void drawField(const GameInfo_t* gameInfo, GameType gameType) {
  clearGameArea(1, 1, FIELD_WIDTH * 2 + 1, FIELD_HEIGHT);

  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (gameInfo->field[y][x]) {
        if (gameType == GameType::SNAKE) {
          if (gameInfo->field[y][x] == 1) {
            setColor(Color::WHITE, Color::RED);
          } else if (gameInfo->field[y][x] < 6) {
            setColor(Color::WHITE, Color::GREEN);
          } else {
            setColor(Color::WHITE, Color::BLUE);
          }
        } else {
          setColor(Color::WHITE, Color::BLUE);
        }
        printAtXY(x * 2 + 1, y + 1, " ");
        printAtXY(x * 2 + 2, y + 1, " ");
        setDefaultColor();
      }
    }
  }
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

void ConsoleView::onInput(GameController& controller) {
  while (running) {
    if (inSelectGame) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    InputEvent input = readKey();
    if (!input.noKey) {
      controller.userInput(input.key, input.hold);
    }
  }
}

InputEvent ConsoleView::readKey() {
  struct KeyBinding {
    unsigned int keyCode;
    Key action;
  };
  const KeyBinding keyBindings[] = {
      {' ', Key::SPACE},  {27, Key::ESC},       {10, Key::ENTER},
      {'P', Key::P},      {'p', Key::P},        {53431, Key::P},
      {53399, Key::P},    {1792836, Key::LEFT}, {1792835, Key::RIGHT},
      {1792833, Key::UP}, {1792834, Key::DOWN}};
  InputEvent event = {Key::ENTER, false, true};
  const int numBindings = sizeof(keyBindings) / sizeof(keyBindings[0]);

  unsigned int code = readUTF8();

  for (int i = 0; i < numBindings && code > 0; i++) {
    if (code == keyBindings[i].keyCode) {
      event.key = keyBindings[i].action;
      event.hold = keyIsHold(code);
      event.noKey = false;
      break;
    }
  }

  return event;
}

ConsoleView::ConsoleView() {
  running = true;

// set console mode input without Enter
#ifdef _WIN32
  HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
  GetConsoleMode(hStdIn, &oldMode);
  SetConsoleMode(hStdIn, oldMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
#else
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif
}

ConsoleView::~ConsoleView() {
  running = false;
  if (keyReader.joinable()) {
    keyReader.join();
  }

// restore settings console
#ifdef _WIN32
  HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
  SetConsoleMode(hStdIn, oldMode);
#else
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

void ConsoleView::startInputThread(GameController& controller) {
  // create thread for reading input
  keyReader = std::thread([this, &controller]() { onInput(controller); });
}

GameType ConsoleView::selectGame() {
  inSelectGame = true;
  GameType selectedGame = GameType::TETRIS;
  currentMenu = Menu::SELECT_GAME;

  clear();
  setColor(Color::GREEN, Color::DEFAULT);
  printAtXY(0, 0, "Select the game:");
  setDefaultColor();
  printAtXY(0, 1, "1. Tetris <=");
  printAtXY(0, 2, "2. Snake");
  moveAtXY(0, 1);
  flushOutput();

  while (true) {
    InputEvent input = readKey();

    if (!input.noKey) {
      if (input.key == Key::UP) {
        selectedGame = GameType::TETRIS;
        printAtXY(10, 1, "<=");
        printAtXY(10, 2, "  ");
        moveAtXY(0, 1);
        flushOutput();
      } else if (input.key == Key::DOWN) {
        selectedGame = GameType::SNAKE;
        printAtXY(10, 2, "<=");
        printAtXY(10, 1, "  ");
        moveAtXY(0, 2);
        flushOutput();
      } else if (input.key == Key::ENTER) {
        break;
      } else if (input.key == Key::ESC) {
        selectedGame = GameType::NONE;
        running = false;
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  inSelectGame = false;
  return selectedGame;
}

void ConsoleView::render(const GameInfo_t& gameInfo, GameStatus gameStatus,
                         GameType gameType) {
  std::lock_guard<std::mutex> lock(renderMutex);
  static GameInfo_t oldGameInfo = {};
  static GameStatus oldGameStatus = GameStatus::PAUSE;
  static GameType oldGameType = GameType::SNAKE;
  static Menu oldMenu = Menu::NONE;
  if (gameInfo == oldGameInfo && gameStatus == oldGameStatus &&
      gameType == oldGameType && oldMenu == currentMenu) {
    return;
  }

  if (gameStatus == GameStatus::INIT && currentMenu != Menu::START) {
    currentMenu = Menu::START;
    startMenu(gameType);
  } else if (gameStatus == GameStatus::INSTRUCTION) {
    currentMenu = Menu::INSTRUCTION;
    if (gameType == GameType::TETRIS) {
      showInstructionsTetris();
    } else if (gameType == GameType::SNAKE) {
      showInstructionsSnake();
    }
  } else if (gameStatus == GameStatus::PAUSE) {
    currentMenu = Menu::PAUSE;
    pauseGame();
  } else if (gameStatus == GameStatus::GAME_OVER) {
    currentMenu = Menu::NONE;
    drawGameOver();
  } else if (gameStatus == GameStatus::WIN) {
    currentMenu = Menu::NONE;
    showCredits();
  } else if (gameStatus == GameStatus::GAME) {
    currentMenu = Menu::NONE;
    drawField(&gameInfo, gameType);
    drawScore(&gameInfo);
    if (gameType == GameType::TETRIS) {
      drawNextShape(&gameInfo);
    }
    moveToStart();
    flushOutput();
  }

  oldMenu = currentMenu;
  oldGameInfo.~GameInfo_t();
  oldGameInfo = gameInfo;
  oldGameStatus = gameStatus;
  oldGameType = gameType;
}
#include "snakeLogic.hpp"

using namespace s21;
using enum SnakeLogic::Field;
using enum SnakeLogic::Direct;

#define DB_ID 212

static bool checkWin(GameInfo_t& gameInfo) {
  bool fieldIsFull = true;
  for (int i = 0; i < FIELD_HEIGHT && fieldIsFull; ++i) {
    for (int j = 0; j < FIELD_WIDTH && fieldIsFull; ++j) {
      if (gameInfo.field[i][j] == 0) {
        fieldIsFull = false;
      }
    }
  }
  return fieldIsFull;
}

static bool checkCollision(int x, int y, GameInfo_t& gameInfo) {
  bool collision = false;

  // check bounds field
  if (x < 0 || x >= FIELD_WIDTH || y < 0 || y >= FIELD_HEIGHT) {
    collision = true;
  }

  // check collision with body snake
  if (!collision && gameInfo.field[y][x] > static_cast<int>(HEAD_DOWN)) {
    collision = true;
  }

  return collision;
}

struct CellSnake {
  int x;
  int y;
  int* cell;
};

static CellSnake getNextBody(CellSnake& cellSnake, GameInfo_t& gameInfo) {
  CellSnake nextBody = {-1, -1, nullptr};

  int targetIndex;
  // if it is snake head, search body with index 1
  if (*cellSnake.cell >= static_cast<int>(HEAD_LEFT) &&
      *cellSnake.cell <= static_cast<int>(HEAD_DOWN)) {
    targetIndex = static_cast<int>(HEAD_DOWN) + 1;
  } else {
    // if it is snake body, search body with index greater than 1
    targetIndex = *cellSnake.cell + 1;
  }

  // check nearest cells
  int dx[] = {-1, 1, 0, 0};
  int dy[] = {0, 0, -1, 1};
  for (int i = 0; i < 4; i++) {
    int newX = cellSnake.x + dx[i];
    int newY = cellSnake.y + dy[i];

    if (newX >= 0 && newX < FIELD_WIDTH && newY >= 0 && newY < FIELD_HEIGHT) {
      if (gameInfo.field[newY][newX] == targetIndex) {
        nextBody.x = newX;
        nextBody.y = newY;
        nextBody.cell = &gameInfo.field[newY][newX];
        break;
      }
    }
  }

  return nextBody;
}

static CellSnake getSnakeHead(GameInfo_t& gameInfo) {
  int headX = -1, headY = -1;
  for (int y = 0; y < FIELD_HEIGHT && headY == -1; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (gameInfo.field[y][x] >= static_cast<int>(HEAD_LEFT) &&
          gameInfo.field[y][x] <= static_cast<int>(HEAD_DOWN)) {
        headX = x;
        headY = y;
        break;
      }
    }
  }
  return {headX, headY, &gameInfo.field[headY][headX]};
}

static void setDirection(GameInfo_t& gameInfo, SnakeLogic::Direct& direct) {
  CellSnake snakeHead = getSnakeHead(gameInfo);
  if (snakeHead.x != -1 && snakeHead.y != -1) {
    SnakeLogic::Field head = static_cast<SnakeLogic::Field>(*snakeHead.cell);
    if (direct == LEFT && head != HEAD_RIGHT) {
      *snakeHead.cell = static_cast<int>(HEAD_LEFT);
    } else if (direct == RIGHT && head != HEAD_LEFT) {
      *snakeHead.cell = static_cast<int>(HEAD_RIGHT);
    } else if (direct == UP && head != HEAD_DOWN) {
      *snakeHead.cell = static_cast<int>(HEAD_UP);
    } else if (direct == DOWN && head != HEAD_UP) {
      *snakeHead.cell = static_cast<int>(HEAD_DOWN);
    } else {
      direct = NONE;
    }
  }
}

static void spawnFood(GameInfo_t& gameInfo) {
  int x, y;
  do {
    x = rand() % FIELD_WIDTH;
    y = rand() % FIELD_HEIGHT;
  } while (gameInfo.field[y][x] != 0);

  gameInfo.field[y][x] = static_cast<int>(FOOD);
}

static void eatFood(GameInfo_t& gameInfo) {
  // search food
  int* cellFood = nullptr;
  for (int y = 0; y < FIELD_HEIGHT && cellFood == nullptr; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (gameInfo.field[y][x] == static_cast<int>(FOOD)) {
        cellFood = &gameInfo.field[y][x];
        break;
      }
    }
  }

  if (cellFood != nullptr) {
    CellSnake head = getSnakeHead(gameInfo);

    // cell with food now is snake head
    *cellFood = *head.cell;

    // increase index of all body parts snake
    for (int y = 0; y < FIELD_HEIGHT; y++) {
      for (int x = 0; x < FIELD_WIDTH; x++) {
        if (gameInfo.field[y][x] > static_cast<int>(HEAD_DOWN)) {
          gameInfo.field[y][x]++;
        }
      }
    }

    // previous snake head now is body
    gameInfo.field[head.y][head.x] = static_cast<int>(HEAD_DOWN) + 1;
  }

  gameInfo.score += 1;

  while (gameInfo.level < 10 && gameInfo.score >= gameInfo.level * 5) {
    gameInfo.level++;
    gameInfo.speed++;
  }

  if (gameInfo.score > gameInfo.high_score) {
    gameInfo.high_score = gameInfo.score;
    GameLogic::saveHighScore(gameInfo.high_score, DB_ID);
  }

  spawnFood(gameInfo);
}

static bool moveSnake(GameInfo_t& gameInfo) {
  bool collision = false;
  CellSnake head = getSnakeHead(gameInfo);

  int newX = head.x, newY = head.y;
  SnakeLogic::Field headField = static_cast<SnakeLogic::Field>(*head.cell);
  if (headField == HEAD_LEFT) {
    newX--;
  } else if (headField == HEAD_RIGHT) {
    newX++;
  } else if (headField == HEAD_UP) {
    newY--;
  } else if (headField == HEAD_DOWN) {
    newY++;
  }
  collision = checkCollision(newX, newY, gameInfo);

  if (!collision) {
    if (gameInfo.field[newY][newX] == static_cast<int>(FOOD)) {
      eatFood(gameInfo);
    } else {
      gameInfo.field[newY][newX] = *head.cell;

      CellSnake currentCell = head;
      while (true) {
        CellSnake nextCell = getNextBody(currentCell, gameInfo);
        if (nextCell.cell == nullptr || nextCell.x == -1 || nextCell.y == -1) {
          break;  // end of snake
        }
        *currentCell.cell = *nextCell.cell;
        currentCell = nextCell;
      }

      //  delete last part body of snake
      if (*currentCell.cell > static_cast<int>(HEAD_DOWN)) {
        *currentCell.cell = 0;
      }
    }
  }

  return collision;
}

static void spawnSnake(GameInfo_t& gameInfo) {
  const int startSize = 4;
  SnakeLogic::Direct direction = static_cast<SnakeLogic::Direct>(rand() % 4);

  int startX = rand() % FIELD_WIDTH;
  int startY = rand() % FIELD_HEIGHT;
  if (direction == LEFT) {
    startX = rand() % (FIELD_WIDTH - 4);
  } else if (direction == RIGHT) {
    startX = rand() % (FIELD_WIDTH - 4) + 3;
  } else if (direction == UP) {
    startY = rand() % (FIELD_HEIGHT - 4);
  } else if (direction == DOWN) {
    startY = rand() % (FIELD_HEIGHT - 4) + 3;
  }

  int headValue = static_cast<int>(HEAD_LEFT);
  for (int i = 0; i < startSize; i++) {
    if (direction == LEFT) {
      gameInfo.field[startY][startX + i] = (i == 0) ? headValue : i + 5;
    } else if (direction == RIGHT) {
      gameInfo.field[startY][startX - i] = (i == 0) ? headValue : i + 5;
    } else if (direction == UP) {
      gameInfo.field[startY + i][startX] = (i == 0) ? headValue : i + 5;
    } else if (direction == DOWN) {
      gameInfo.field[startY - i][startX] = (i == 0) ? headValue : i + 5;
    }
  }

  setDirection(gameInfo, direction);
}

static void initGame(GameInfo_t& gameInfo) {
  for (int i = 0; i < FIELD_HEIGHT; i++) {
    for (int j = 0; j < FIELD_WIDTH; j++) {
      gameInfo.field[i][j] = 0;
    }
  }

  gameInfo.high_score = GameLogic::loadHighScore(DB_ID);
  gameInfo.score = 0;
  gameInfo.level = 1;
  gameInfo.speed = 1;
  gameInfo.pause = 0;

  spawnSnake(gameInfo);
  spawnFood(gameInfo);
}

struct ActionParams {
  UserAction_t action;
  bool hold;
  GameStatus& gameStatus;
  GameInfo_t& gameInfo;
};

static bool movingAction(ActionParams& AP) {
  bool isMovingSnake = false;
  SnakeLogic::Direct direct = NONE;
  if (AP.action == UserAction_t::Left) {
    direct = LEFT;
  } else if (AP.action == UserAction_t::Right) {
    direct = RIGHT;
  } else if (AP.action == UserAction_t::Up) {
    direct = UP;
  } else if (AP.action == UserAction_t::Down) {
    direct = DOWN;
  }

  if (direct != NONE) {
    setDirection(AP.gameInfo, direct);
  }

  if (direct != NONE || AP.action == UserAction_t::Action) {
    if (moveSnake(AP.gameInfo)) {
      AP.gameStatus = GameStatus::GAME_OVER;
      AP.gameInfo.pause = 1;
    } else if (checkWin(AP.gameInfo)) {
      AP.gameStatus = GameStatus::WIN;
      AP.gameInfo.pause = 1;
    }
    isMovingSnake = true;
  }

  return isMovingSnake;
}

static bool gameAction(ActionParams& AP) {
  bool isMovingSnake = false;
  using UA = UserAction_t;
  if (AP.hold && (AP.action == UA::Pause || AP.action == UA::Terminate)) {
    return isMovingSnake;
  }

  if (AP.action == UserAction_t::Pause) {
    AP.gameStatus = GameStatus::PAUSE;
    AP.gameInfo.pause = 1;
  } else if (AP.action == UA::Terminate) {
    AP.gameStatus = GameStatus::INIT;
    AP.gameInfo.pause = 1;
  } else if (!AP.gameInfo.pause) {
    isMovingSnake = movingAction(AP);
  }

  return isMovingSnake;
}

static void gameOverAction(ActionParams& AP) {
  if (AP.action == UserAction_t::Start) {
    initGame(AP.gameInfo);
    AP.gameStatus = GameStatus::GAME;
  } else if (AP.action == UserAction_t::Terminate) {
    AP.gameStatus = GameStatus::INIT;
  }
}

static void pauseAction(ActionParams& AP) {
  switch (AP.action) {
    case UserAction_t::Pause:
    case UserAction_t::Start:
    case UserAction_t::Terminate:
      AP.gameStatus = GameStatus::GAME;
      AP.gameInfo.pause = 0;
      break;
    default:
      break;
  }
}

static void instructionAction(ActionParams& AP) {
  if (AP.action == UserAction_t::Terminate) {
    AP.gameStatus = GameStatus::INIT;
  } else if (AP.action == UserAction_t::Start) {
    initGame(AP.gameInfo);
    AP.gameStatus = GameStatus::GAME;
  }
}

static void initAction(ActionParams& AP) {
  if (AP.action == UserAction_t::Up) {
    AP.gameStatus = GameStatus::INSTRUCTION;
  } else if (AP.action == UserAction_t::Start) {
    initGame(AP.gameInfo);
    AP.gameStatus = GameStatus::GAME;
  }
}

void SnakeLogic::userInput(UserAction_t action, bool hold) {
  std::lock_guard<std::mutex> lock(gameTickMutex);
  ActionParams actionParams = {action, hold, currentGameStatus, gameInfo};

  switch (currentGameStatus) {
    case GameStatus::INIT:
      initAction(actionParams);
      break;
    case GameStatus::INSTRUCTION:
      instructionAction(actionParams);
      break;
    case GameStatus::PAUSE:
      pauseAction(actionParams);
      break;
    case GameStatus::GAME_OVER:
      gameOverAction(actionParams);
      break;
    case GameStatus::WIN:
      gameOverAction(actionParams);
      break;
    case GameStatus::GAME:
      if (gameAction(actionParams)) {
        lastTickTime = std::chrono::high_resolution_clock::now();
      }
      break;
  }
}

GameInfo_t SnakeLogic::updateCurrentState() { return gameInfo; }

void SnakeLogic::gameTick() {
  std::lock_guard<std::mutex> lock(gameTickMutex);

  if (currentGameStatus == GameStatus::GAME && !gameInfo.pause) {
    if (moveSnake(gameInfo)) {
      currentGameStatus = GameStatus::GAME_OVER;
      gameInfo.pause = 1;
    } else if (checkWin(gameInfo)) {
      currentGameStatus = GameStatus::WIN;
      gameInfo.pause = 1;
    }
  }

  lastTickTime = std::chrono::high_resolution_clock::now();
}

SnakeLogic::SnakeLogic() {
  srand(time(nullptr));
  gameInfo.field = new int*[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; i++) {
    gameInfo.field[i] = new int[FIELD_WIDTH]();
  }
  gameInfo.score = 0;
  gameInfo.high_score = 0;
  gameInfo.level = 1;
  gameInfo.speed = 1;
  gameInfo.pause = 1;
}

SnakeLogic::~SnakeLogic() {
  if (gameInfo.field != nullptr) {
    for (int i = 0; i < FIELD_HEIGHT; i++) {
      delete[] gameInfo.field[i];
    }
    delete[] gameInfo.field;
    gameInfo.field = nullptr;
  }
}
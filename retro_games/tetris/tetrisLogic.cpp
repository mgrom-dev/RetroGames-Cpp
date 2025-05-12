#include "tetrisLogic.hpp"

#include <cstdlib>

using namespace s21;

#define DB_ID 211
#define SCORE 0  // for test game

//
// ============================================================================
// Functions for initializing shapes
// ============================================================================

typedef enum {
  Random,
  Stick,      // I
  Square,     // O
  TShape,     // T
  LShape,     // L
  LShapeRev,  // revert L
  ZShape,     // Z
  ZShapeRev   // revert Z
} ShapeType;

typedef struct {
  int** grid;
  int width;
  int height;
  int x;
  int y;
} Shape;

static void initializeStick(Shape* shape) {
  shape->width = 1;
  shape->height = 4;
  for (int i = 0; i < 4; i++) {
    shape->grid[i][0] = 1;
  }
}

static void initializeSquare(Shape* shape) {
  shape->width = 2;
  shape->height = 2;
  shape->grid[0][0] = 1;
  shape->grid[0][1] = 1;
  shape->grid[1][0] = 1;
  shape->grid[1][1] = 1;
}

static void initializeTShape(Shape* shape) {
  shape->width = 3;
  shape->height = 2;
  shape->grid[0][1] = 1;
  shape->grid[1][0] = 1;
  shape->grid[1][1] = 1;
  shape->grid[1][2] = 1;
}

static void initializeLShape(Shape* shape) {
  shape->width = 3;
  shape->height = 2;
  shape->grid[0][0] = 1;
  shape->grid[1][0] = 1;
  shape->grid[1][1] = 1;
  shape->grid[1][2] = 1;
}

static void initializeZShape(Shape* shape) {
  shape->width = 3;
  shape->height = 2;
  shape->grid[0][0] = 1;
  shape->grid[0][1] = 1;
  shape->grid[1][1] = 1;
  shape->grid[1][2] = 1;
}

static void initializeLShapeRev(Shape* shape) {
  shape->width = 3;
  shape->height = 2;
  shape->grid[0][2] = 1;
  shape->grid[1][0] = 1;
  shape->grid[1][1] = 1;
  shape->grid[1][2] = 1;
}

static void initializeZShapeRev(Shape* shape) {
  shape->width = 3;
  shape->height = 2;
  shape->grid[0][1] = 1;
  shape->grid[0][2] = 1;
  shape->grid[1][0] = 1;
  shape->grid[1][1] = 1;
}

void (*initializeFunctions[])(Shape*) = {nullptr,  // for Random
                                         initializeStick,
                                         initializeSquare,
                                         initializeTShape,
                                         initializeLShape,
                                         initializeLShapeRev,
                                         initializeZShape,
                                         initializeZShapeRev};

static Shape* createShape(ShapeType shapeType) {
  Shape* createdShape = (Shape*)malloc(sizeof(Shape));
  createdShape->grid = (int**)malloc(NEXT_HEIGHT * sizeof(int*));
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    createdShape->grid[i] = (int*)calloc(NEXT_WIDTH, sizeof(int));
  }
  createdShape->x = 0;
  createdShape->y = 0;

  if (shapeType == Random) {
    int countShapes =
        sizeof(initializeFunctions) / sizeof(initializeFunctions[0]);
    shapeType = (ShapeType)(1 + (rand() % (countShapes - 1)));
  }

  if (shapeType >= Stick && shapeType <= ZShapeRev) {
    initializeFunctions[shapeType](createdShape);
  }

  return createdShape;
}

static void destroyShape(Shape*& shape) {
  if (shape != nullptr) {
    for (int i = 0; i < NEXT_HEIGHT; i++) {
      free(shape->grid[i]);
    }
    free(shape->grid);
    free(shape);
    shape = nullptr;
  }
}

//
// ============================================================================
// End functions for initializing shapes
// ============================================================================

//
// ============================================================================
// Functions for storing game information
// ============================================================================

static Shape* createShapeCopy(const Shape* shape) {
  Shape* newShape = (Shape*)malloc(sizeof(Shape));

  newShape->width = shape->width;
  newShape->height = shape->height;
  newShape->x = shape->x;
  newShape->y = shape->y;

  newShape->grid = (int**)malloc(NEXT_HEIGHT * sizeof(int*));
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    newShape->grid[i] = (int*)malloc(NEXT_WIDTH * sizeof(int));
    for (int j = 0; j < NEXT_WIDTH; j++) {
      newShape->grid[i][j] = shape->grid[i][j];
    }
  }

  return newShape;
}

// return active figure
static Shape** getCurrentShape() {
  static Shape* currentShape = nullptr;
  return &currentShape;
}

static void setCurrentShape(Shape* shape) {
  Shape** currentShapePtr = getCurrentShape();
  *currentShapePtr = createShapeCopy(shape);
}

// return next shape
static Shape** getNextShape() {
  static Shape* nextShape = nullptr;
  return &nextShape;
}

static void setNextShape(Shape* shape) {
  Shape** nextShapePtr = getNextShape();
  *nextShapePtr = createShapeCopy(shape);
}

//
// ============================================================================
// End functions for storing game information
// ============================================================================

//
// ============================================================================
// Functions for moving shapes
// ============================================================================

typedef enum { RotateRight, RotateLeft } RotateSide;

static void rotateShapeSimple(Shape* shape, RotateSide side) {
  // temp grid for rotate
  int** newGrid = (int**)malloc(NEXT_HEIGHT * sizeof(int*));
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    newGrid[i] = (int*)calloc(NEXT_WIDTH, sizeof(int));
  }

  // make rotate
  for (int i = 0; i < shape->height; i++) {
    for (int j = 0; j < shape->width; j++) {
      if (side == RotateRight) {
        newGrid[j][shape->height - 1 - i] = shape->grid[i][j];
      } else {
        newGrid[shape->width - 1 - j][i] = shape->grid[i][j];
      }
    }
  }

  // refresh origin shape
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    free(shape->grid[i]);
  }
  free(shape->grid);
  shape->grid = newGrid;
  int temp = shape->width;
  shape->width = shape->height;
  shape->height = temp;
}

static bool checkCollision(Shape* shape, int** field) {
  bool collision = false;
  for (int i = 0; i < shape->height && !collision; i++) {
    for (int j = 0; j < shape->width && !collision; j++) {
      if (!shape->grid[i][j]) continue;
      int newX = shape->x + j;
      int newY = shape->y - (shape->height - i - 1);

      if (newY < 0) newY = 0;

      // check bound and field collision
      bool boundX = newX >= FIELD_WIDTH || newX < 0;
      bool boundY = newY >= FIELD_HEIGHT || newY < 0;
      if (boundX || boundY || field[newY][newX] != 0) {
        collision = true;
      }
    }
  }
  return collision;
}

// add, or remove shape from field
static void updateShapeOnField(Shape* shape, int** field, bool add) {
  for (int i = shape->height - 1; i >= 0; i--) {
    for (int j = 0; j < shape->width; j++) {
      if (!shape->grid[i][j]) continue;

      // position on field
      int x = shape->x + j;
      int y = shape->y - (shape->height - i - 1);

      // check bounds of field
      if (y >= 0 && y < FIELD_HEIGHT && x >= 0 && x < FIELD_WIDTH) {
        field[y][x] = add ? shape->grid[i][j] : 0;
      }
    }
  }
}

static bool moveShape(int dx, int dy, GameInfo_t& gameInfo) {
  bool isCollision = false;
  Shape* currentShape = *getCurrentShape();
  updateShapeOnField(currentShape, gameInfo.field, false);

  currentShape->x += dx;
  currentShape->y += dy;

  if (checkCollision(currentShape, gameInfo.field)) {
    currentShape->x -= dx;
    currentShape->y -= dy;
    isCollision = true;
  }

  updateShapeOnField(currentShape, gameInfo.field, true);
  return isCollision;
}

static void spawnNewShape(GameInfo_t& gameInfo) {
  Shape* tempShape = nullptr;
  destroyShape(*getCurrentShape());

  if (*getNextShape() == nullptr) {
    tempShape = createShape(Random);
    setCurrentShape(tempShape);
    destroyShape(tempShape);
  } else {
    setCurrentShape(*getNextShape());
    destroyShape(*getNextShape());
  }

  tempShape = createShape(Random);
  setNextShape(tempShape);
  destroyShape(tempShape);

  // Random rotate shape from 0 to 3
  int rotations = rand() % 4;
  for (int i = 0; i < rotations; i++) {
    rotateShapeSimple(*getNextShape(), RotateRight);
  }
  gameInfo.next = (*getNextShape())->grid;

  // Random position on the X-axis
  int randomX = rand() % (FIELD_WIDTH - (*getCurrentShape())->width);
  (*getCurrentShape())->x = randomX;
  updateShapeOnField((*getCurrentShape()), gameInfo.field, true);
}

//
// ============================================================================
// End functions for moving shapes
// ============================================================================

static void gameOver(GameInfo_t& gameInfo, GameStatus& gameStatus) {
  gameInfo.pause = 1;
  gameStatus = GameStatus::GAME_OVER;
}

static int removeClearLines(GameInfo_t& gameInfo) {
  int removedLines = 0;

  for (int y = FIELD_HEIGHT - 1; y >= 0; y--) {
    bool isLineClear = true;
    for (int x = 0; x < FIELD_WIDTH; x++) {
      isLineClear &= gameInfo.field[y][x] != 0;
    }

    if (isLineClear) {
      removedLines++;

      for (int j = y; j > 0; j--) {
        for (int x = 0; x < FIELD_WIDTH; x++) {
          gameInfo.field[j][x] = gameInfo.field[j - 1][x];
        }
      }
      y++;
    }
  }

  return removedLines;
}

static void clearFullLines(GameInfo_t& gameInfo, GameStatus& gameStatus) {
  int clearedLines = removeClearLines(gameInfo);

  switch (clearedLines) {
    case 1:
      gameInfo.score += 100;
      break;
    case 2:
      gameInfo.score += 300;
      break;
    case 3:
      gameInfo.score += 700;
      break;
    case 4:
      gameInfo.score += 1500;
      break;
  }

  if (gameInfo.score > gameInfo.high_score) {
    gameInfo.high_score = gameInfo.score;
    GameLogic::saveHighScore(gameInfo.high_score, DB_ID);
  }

  while (gameInfo.score >= gameInfo.level * 600) {
    gameInfo.level++;
    gameInfo.speed++;
  }

  if (gameInfo.level > 10) {
    gameStatus = GameStatus::WIN;
    gameInfo.pause = 1;
  }
}

static void startGame(GameStatus& gameStatus, GameInfo_t& gameInfo) {
  gameStatus = GameStatus::GAME;

  for (int i = 0; i < FIELD_HEIGHT; i++) {
    for (int j = 0; j < FIELD_WIDTH; j++) {
      gameInfo.field[i][j] = 0;
    }
  }
  gameInfo.high_score = GameLogic::loadHighScore(DB_ID);
  gameInfo.score = SCORE;
  gameInfo.level = 1;
  gameInfo.speed = 1;
  gameInfo.pause = 0;

  spawnNewShape(gameInfo);
}

static void rotateShape(bool hold, GameInfo_t& gameInfo) {
  (void)hold;
  Shape* currentShape = *getCurrentShape();
  int oldX = currentShape->x;
  updateShapeOnField(currentShape, gameInfo.field, false);
  rotateShapeSimple(currentShape, RotateRight);
  while (currentShape->x + currentShape->width > FIELD_WIDTH) {
    currentShape->x--;
  }

  // collision with other shapes on field
  if (checkCollision(currentShape, gameInfo.field)) {
    rotateShapeSimple(currentShape, RotateLeft);
    currentShape->x = oldX;
  }

  updateShapeOnField(currentShape, gameInfo.field, true);
}

TetrisLogic::TetrisLogic() {
  srand(time(nullptr));
  gameInfo.field = nullptr;
  gameInfo.next = nullptr;
  gameInfo.field = (int**)malloc(FIELD_HEIGHT * sizeof(int*));
  for (int i = 0; i < FIELD_HEIGHT; i++) {
    gameInfo.field[i] = (int*)calloc(FIELD_WIDTH, sizeof(int));
  }
  gameInfo.score = 0;
  gameInfo.high_score = 0;
  gameInfo.level = 1;
  gameInfo.speed = 1;
  gameInfo.pause = 1;
}

TetrisLogic::~TetrisLogic() {
  if (gameInfo.field != nullptr) {
    for (int i = 0; i < FIELD_HEIGHT; i++) {
      free(gameInfo.field[i]);
    }
    free(gameInfo.field);
    gameInfo.field = nullptr;
  }

  destroyShape(*getCurrentShape());
  destroyShape(*getNextShape());
  gameInfo.next = nullptr;
}

struct ActionParams {
  UserAction_t action;
  bool hold;
  GameStatus& gameStatus;
  GameInfo_t& gameInfo;
};

static bool gameAction(ActionParams& AP) {
  bool isGameTick = false;
  using UA = UserAction_t;
  if (AP.hold && (AP.action == UA::Pause || AP.action == UA::Terminate)) {
    return isGameTick;
  }

  if (AP.action == UA::Pause) {
    AP.gameStatus = GameStatus::PAUSE;
    AP.gameInfo.pause = 1;
  } else if (AP.action == UA::Terminate) {
    AP.gameStatus = GameStatus::INIT;
    AP.gameInfo.pause = 1;
  } else if (!AP.gameInfo.pause) {
    if (AP.action == UA::Left) {
      moveShape(-1, 0, AP.gameInfo);
    } else if (AP.action == UA::Right) {
      moveShape(1, 0, AP.gameInfo);
    } else if (AP.action == UA::Action || AP.action == UA::Up) {
      rotateShape(AP.hold, AP.gameInfo);
    } else if (AP.action == UA::Down) {
      isGameTick = true;
    }
  }

  return isGameTick;
}

static void gameOverAction(ActionParams& AP) {
  if (AP.action == UserAction_t::Start) {
    startGame(AP.gameStatus, AP.gameInfo);
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
    startGame(AP.gameStatus, AP.gameInfo);
  }
}

static void initAction(ActionParams& AP) {
  if (AP.action == UserAction_t::Up) {
    AP.gameStatus = GameStatus::INSTRUCTION;
  } else if (AP.action == UserAction_t::Start) {
    startGame(AP.gameStatus, AP.gameInfo);
  }
}

void TetrisLogic::userInput(UserAction_t action, bool hold) {
  ActionParams actionParams = {action, hold, currentGameStatus, gameInfo};

  switch (currentGameStatus) {
    case GameStatus::INIT: {
      std::lock_guard<std::mutex> lock(gameTickMutex);
      initAction(actionParams);
    } break;
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
        gameTick();
      }
      break;
  }
}

GameInfo_t TetrisLogic::updateCurrentState() { return gameInfo; }

void TetrisLogic::gameTick() {
  std::lock_guard<std::mutex> lock(gameTickMutex);

  GameStatus GS = currentGameStatus;
  if (GS != GameStatus::GAME) return;
  if (moveShape(0, 1, gameInfo)) {
    Shape* currentShape = *getCurrentShape();
    if (currentShape->y - currentShape->height < 0) {
      gameOver(gameInfo, currentGameStatus);
    } else {
      clearFullLines(gameInfo, currentGameStatus);
      spawnNewShape(gameInfo);
    }
  }

  lastTickTime = std::chrono::high_resolution_clock::now();
}
#include "testTetris.hpp"

using namespace s21;

TEST_F(TetrisLogicTest, constructor) {
  // constructor
  TetrisLogic* tetris = new TetrisLogic();
  GameInfo_t gameData = tetris->updateCurrentState();
  EXPECT_EQ(gameData.level, 1);
  EXPECT_EQ(gameData.speed, 1);
  EXPECT_EQ(gameData.pause, 1);
  EXPECT_EQ(gameData.score, 0);
  EXPECT_EQ(gameData.high_score, 0);
  delete tetris;
}

TEST_F(TetrisLogicTest, init) {
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(gameData.level, 1);
  EXPECT_EQ(gameData.speed, 1);
  EXPECT_EQ(gameData.pause, 1);
  EXPECT_EQ(gameData.score, 0);
  EXPECT_EQ(gameData.high_score, 0);
}

TEST_F(TetrisLogicTest, instruction) {
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INSTRUCTION);
  userInput(UserAction_t::Terminate, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INSTRUCTION);
  userInput(UserAction_t::Start, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
}

TEST_F(TetrisLogicTest, start_game) {
  userInput(UserAction_t::Start, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(gameData.level, 1);
  EXPECT_EQ(gameData.speed, 1);
  EXPECT_EQ(gameData.pause, 0);
  EXPECT_EQ(gameData.score, 0);
  EXPECT_EQ(gameData.high_score, 0);
  userInput(UserAction_t::Terminate, true);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
}

TEST_F(TetrisLogicTest, pause_game) {
  userInput(UserAction_t::Start, false);
  userInput(UserAction_t::Pause, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::PAUSE);
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::PAUSE);
  userInput(UserAction_t::Pause, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
}

TEST_F(TetrisLogicTest, game_over) {
  userInput(UserAction_t::Start, false);
  while (getCurrentGameStatus() != GameStatus::GAME_OVER) {
    gameTick();
  }
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME_OVER);
  userInput(UserAction_t::Start, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  while (getCurrentGameStatus() != GameStatus::GAME_OVER) {
    gameTick();
  }
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME_OVER);
  userInput(UserAction_t::Terminate, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
}

TEST_F(TetrisLogicTest, out_game) {
  userInput(UserAction_t::Start, false);
  userInput(UserAction_t::Terminate, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
  EXPECT_EQ(updateCurrentState().pause, 1);
}

TEST_F(TetrisLogicTest, level_up) {
  userInput(UserAction_t::Start, false);

  // fill bottom 3 lines
  for (int y = 17; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      gameInfo.field[y][x] = 1;
    }
  }

  for (int i = 0; i < 20; i++) {
    gameTick();
  }

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  EXPECT_EQ(gameData.score, 700);
  EXPECT_EQ(gameData.high_score, 700);
  EXPECT_EQ(gameData.level, 2);
  EXPECT_EQ(gameData.speed, 2);
}

TEST_F(TetrisLogicTest, high_score) {
  userInput(UserAction_t::Start, false);

  // fill bottom 4 lines
  for (int y = 16; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      gameInfo.field[y][x] = 1;
    }
  }

  for (int i = 0; i < 20; i++) {
    gameTick();
  }

  // new game
  userInput(UserAction_t::Terminate, false);
  userInput(UserAction_t::Start, false);

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  EXPECT_EQ(gameData.score, 0);
  EXPECT_EQ(gameData.high_score, 1500);
  EXPECT_EQ(gameData.level, 1);
  EXPECT_EQ(gameData.speed, 1);
}

TEST_F(TetrisLogicTest, move_left) {
  bool testMinLeft = false;
  bool simpleTest = false;
  while (!testMinLeft || !simpleTest) {
    userInput(UserAction_t::Start, false);
    userInput(UserAction_t::Down, false);
    userInput(UserAction_t::Down, false);
    userInput(UserAction_t::Down, false);

    int minRow = -1;
    int minX = -1;
    for (int y = 0; y < FIELD_HEIGHT; y++) {
      for (int x = 0; x < FIELD_WIDTH; x++) {
        if (gameInfo.field[y][x] != 0) {
          if (minX == -1 || minX > x) {
            minX = x;
            minRow = y;
          }
        }
      }
    }

    int originalField[FIELD_WIDTH];
    for (int i = 0; i < FIELD_WIDTH; i++) {
      originalField[i] = gameInfo.field[minRow][i];
    }

    userInput(UserAction_t::Left, false);

    if (originalField[0] == 1) {  // shape in min left position
      for (int i = 0; i < FIELD_WIDTH; i++) {
        EXPECT_EQ(originalField[i], gameInfo.field[minRow][i]);
      }
      testMinLeft = true;
    } else {  // shape should move
      for (int i = 1; i < FIELD_WIDTH; i++) {
        EXPECT_EQ(originalField[i], gameInfo.field[minRow][i - 1]);
      }
      EXPECT_EQ(gameInfo.field[minRow][FIELD_WIDTH - 1],
                0);  // last cell must empty
      simpleTest = true;
    }
    if (!testMinLeft || !simpleTest) {
      userInput(UserAction_t::Action, false);
      userInput(UserAction_t::Terminate, false);
    }
  }
}

TEST_F(TetrisLogicTest, move_right) {
  userInput(UserAction_t::Start, false);

  int originalField[FIELD_WIDTH];
  for (int i = 0; i < FIELD_WIDTH; i++) originalField[i] = gameInfo.field[0][i];

  userInput(UserAction_t::Right, false);

  for (int i = FIELD_WIDTH - 2; i >= 0; i--) {
    EXPECT_EQ(originalField[i], gameInfo.field[0][i + 1]);
  }
  EXPECT_EQ(gameInfo.field[0][0], 0);  // first cell must empty
}

TEST_F(TetrisLogicTest, move_down) {
  userInput(UserAction_t::Start, false);

  int originalField[FIELD_WIDTH];
  for (int i = 0; i < FIELD_WIDTH; i++) originalField[i] = gameInfo.field[0][i];

  userInput(UserAction_t::Down, false);

  for (int i = FIELD_WIDTH - 1; i > 0; i--) {
    EXPECT_EQ(originalField[i], gameInfo.field[1][i]);
  }
}

TEST_F(TetrisLogicTest, gameTick) {
  userInput(UserAction_t::Start, false);

  for (int i = 0; i < FIELD_HEIGHT; i++) gameTick();

  int bottomLineFilledCells = 0;

  for (int i = 0; i < FIELD_WIDTH; i++) {
    if (gameInfo.field[FIELD_HEIGHT - 1][i]) bottomLineFilledCells++;
  }

  EXPECT_GT(bottomLineFilledCells, 0);
}

static void getWidthFigureUpper(int** field, int& width, int& height) {
  int minY, maxY, minX, maxX;
  minY = maxY = minX = maxX = width = height = -1;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (field[y][x] > 0) {
        if (minY == -1 || minY > y) minY = y;
        if (maxY == -1 || maxY < y) maxY = y;
        if (minX == -1 || minX > x) minX = x;
        if (maxX == -1 || maxX < x) maxX = x;
      }
    }
  }
  if (maxX != -1 && minX != -1) {
    width = maxX - minX;
  }
  if (maxY != -1 && minY != -1) {
    height = maxY - minY;
  }
}

TEST_F(TetrisLogicTest, key_up) {
  userInput(UserAction_t::Start, false);

  gameTick();
  gameTick();
  gameTick();

  int oldWidth, oldHeight;
  getWidthFigureUpper(gameInfo.field, oldWidth, oldHeight);

  userInput(UserAction_t::Up, false);

  int newWidth, newHeight;
  getWidthFigureUpper(gameInfo.field, newWidth, newHeight);

  EXPECT_EQ(newWidth, oldHeight);
  EXPECT_EQ(newHeight, oldWidth);
}

TEST_F(TetrisLogicTest, action) {
  userInput(UserAction_t::Start, false);

  gameTick();
  gameTick();
  gameTick();

  int oldWidth, oldHeight;
  getWidthFigureUpper(gameInfo.field, oldWidth, oldHeight);

  userInput(UserAction_t::Action, false);

  int newWidth, newHeight;
  getWidthFigureUpper(gameInfo.field, newWidth, newHeight);

  EXPECT_EQ(newWidth, oldHeight);
  EXPECT_EQ(newHeight, oldWidth);

  for (int i = 0; i < 10; i++) {
    userInput(UserAction_t::Right, false);
  }
  userInput(UserAction_t::Action, false);
  getWidthFigureUpper(gameInfo.field, newWidth, newHeight);
  EXPECT_EQ(newWidth, oldWidth);
  EXPECT_EQ(newHeight, oldHeight);
}

TEST_F(TetrisLogicTest, clear_line_1) {
  userInput(UserAction_t::Start, false);

  // fill bottom 1 lines
  for (int y = 19; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      gameInfo.field[y][x] = 1;
    }
  }

  for (int i = 0; i < 20; i++) {
    gameTick();
  }

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  EXPECT_EQ(gameData.score, 100);
}

TEST_F(TetrisLogicTest, clear_line_2) {
  userInput(UserAction_t::Start, false);

  // fill bottom 2 lines
  for (int y = 18; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      gameInfo.field[y][x] = 1;
    }
  }

  for (int i = 0; i < 20; i++) {
    gameTick();
  }

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  EXPECT_EQ(gameData.score, 300);
}

TEST_F(TetrisLogicTest, clear_line_3) {
  userInput(UserAction_t::Start, false);

  // fill bottom 3 lines
  for (int y = 17; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      gameInfo.field[y][x] = 1;
    }
  }

  for (int i = 0; i < 20; i++) {
    gameTick();
  }

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  EXPECT_EQ(gameData.score, 700);
}

TEST_F(TetrisLogicTest, clear_line_4) {
  userInput(UserAction_t::Start, false);

  // fill bottom 4 lines
  for (int y = 16; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      gameInfo.field[y][x] = 1;
    }
  }

  for (int i = 0; i < 20; i++) {
    gameTick();
  }

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  EXPECT_EQ(gameData.score, 1500);
}

TEST_F(TetrisLogicTest, win) {
  userInput(UserAction_t::Start, false);

  gameInfo.score = 6000;
  // fill bottom 4 lines
  for (int y = 16; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      gameInfo.field[y][x] = 1;
    }
  }

  for (int i = 0; i < 20; i++) {
    gameTick();
  }

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::WIN);
  EXPECT_EQ(gameData.score, 7500);
  EXPECT_EQ(gameData.pause, 1);
  userInput(UserAction_t::Terminate, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
}
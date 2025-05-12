#include "testSnake.hpp"

using namespace s21;

TEST_F(SnakeLogicTest, constructor_1) {
  SnakeLogic* snake = new SnakeLogic();
  GameInfo_t gameData = snake->updateCurrentState();
  EXPECT_EQ(gameData.level, 1);
  EXPECT_EQ(gameData.speed, 1);
  EXPECT_EQ(gameData.pause, 1);
  EXPECT_EQ(gameData.score, 0);
  EXPECT_EQ(gameData.high_score, 0);
  delete snake;
}

TEST_F(SnakeLogicTest, init) {
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(gameData.level, 1);
  EXPECT_EQ(gameData.speed, 1);
  EXPECT_EQ(gameData.pause, 1);
  EXPECT_EQ(gameData.score, 0);
  EXPECT_EQ(gameData.high_score, 0);
}

TEST_F(SnakeLogicTest, instruction) {
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INSTRUCTION);
  userInput(UserAction_t::Start, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
}

TEST_F(SnakeLogicTest, start_game) {
  userInput(UserAction_t::Start, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(gameData.level, 1);
  EXPECT_EQ(gameData.speed, 1);
  EXPECT_EQ(gameData.pause, 0);
  EXPECT_EQ(gameData.score, 0);
  EXPECT_EQ(gameData.high_score, 0);
}

TEST_F(SnakeLogicTest, start_game_1) {
  for (int i = 0; i < 20; i++) {
    userInput(UserAction_t::Start, false);
    EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
    userInput(UserAction_t::Terminate, false);
    EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
  }
  userInput(UserAction_t::Start, false);
  userInput(UserAction_t::Terminate, true);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
}

TEST_F(SnakeLogicTest, pause_game) {
  userInput(UserAction_t::Start, false);
  userInput(UserAction_t::Pause, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::PAUSE);
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::PAUSE);
  userInput(UserAction_t::Pause, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
}

TEST_F(SnakeLogicTest, game_over) {
  userInput(UserAction_t::Start, false);
  for (int i = 0; i < 21; i++) {
    gameTick();
    if (getCurrentGameStatus() == GameStatus::GAME_OVER) {
      break;
    }
  }
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME_OVER);

  // restart game
  userInput(UserAction_t::Start, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  for (int i = 0; i < 21; i++) {
    userInput(UserAction_t::Left, false);
    if (getCurrentGameStatus() == GameStatus::GAME_OVER) {
      break;
    }
  }
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME_OVER);
  userInput(UserAction_t::Terminate, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
}

TEST_F(SnakeLogicTest, level_up) {
  userInput(UserAction_t::Start, false);
  fillUpperFieldFood();

  // eat 5 food
  for (int i = 0; i < 5; i++) {
    gameTick();
  }

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  EXPECT_EQ(gameData.score, 5);
  EXPECT_EQ(gameData.high_score, 5);
  EXPECT_EQ(gameData.level, 2);
  EXPECT_EQ(gameData.speed, 2);
}

TEST_F(SnakeLogicTest, out_game) {
  userInput(UserAction_t::Start, false);
  userInput(UserAction_t::Terminate, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
  EXPECT_EQ(updateCurrentState().pause, 1);
}

TEST_F(SnakeLogicTest, high_score) {
  userInput(UserAction_t::Start, false);
  fillUpperFieldFood();

  // eat 5 food
  for (int i = 0; i < 5; i++) {
    gameTick();
  }

  // new game
  userInput(UserAction_t::Terminate, false);
  userInput(UserAction_t::Start, false);

  GameInfo_t gameData = updateCurrentState();
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME);
  EXPECT_EQ(gameData.score, 0);
  EXPECT_EQ(gameData.high_score, 5);
  EXPECT_EQ(gameData.level, 1);
  EXPECT_EQ(gameData.speed, 1);
}

TEST_F(SnakeLogicTest, move_left) {
  using enum SnakeLogic::Field;
  userInput(UserAction_t::Start, false);

  setSnakeToCenterFieldHeadUp();
  userInput(UserAction_t::Left, false);
  EXPECT_EQ(gameInfo.field[8][3], static_cast<int>(HEAD_LEFT));

  setSnakeToCenterFieldHeadDown();
  userInput(UserAction_t::Left, false);
  EXPECT_EQ(gameInfo.field[11][3], static_cast<int>(HEAD_LEFT));

  setSnakeToCenterFieldHeadLeft();
  userInput(UserAction_t::Left, false);
  EXPECT_EQ(gameInfo.field[9][2], static_cast<int>(HEAD_LEFT));

  setSnakeToCenterFieldHeadRight();
  userInput(UserAction_t::Left, false);
  EXPECT_EQ(gameInfo.field[9][6], static_cast<int>(HEAD_RIGHT));
}

TEST_F(SnakeLogicTest, move_right) {
  using enum SnakeLogic::Field;
  userInput(UserAction_t::Start, false);

  setSnakeToCenterFieldHeadUp();
  userInput(UserAction_t::Right, false);
  EXPECT_EQ(gameInfo.field[8][5], static_cast<int>(HEAD_RIGHT));

  setSnakeToCenterFieldHeadDown();
  userInput(UserAction_t::Right, false);
  EXPECT_EQ(gameInfo.field[11][5], static_cast<int>(HEAD_RIGHT));

  setSnakeToCenterFieldHeadLeft();
  userInput(UserAction_t::Right, false);
  EXPECT_EQ(gameInfo.field[9][3], static_cast<int>(HEAD_LEFT));

  setSnakeToCenterFieldHeadRight();
  userInput(UserAction_t::Right, false);
  EXPECT_EQ(gameInfo.field[9][7], static_cast<int>(HEAD_RIGHT));
}

TEST_F(SnakeLogicTest, move_up) {
  using enum SnakeLogic::Field;
  userInput(UserAction_t::Start, false);

  setSnakeToCenterFieldHeadUp();
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(gameInfo.field[7][4], static_cast<int>(HEAD_UP));

  setSnakeToCenterFieldHeadDown();
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(gameInfo.field[11][4], static_cast<int>(HEAD_DOWN));

  setSnakeToCenterFieldHeadLeft();
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(gameInfo.field[8][3], static_cast<int>(HEAD_UP));

  setSnakeToCenterFieldHeadRight();
  userInput(UserAction_t::Up, false);
  EXPECT_EQ(gameInfo.field[8][6], static_cast<int>(HEAD_UP));
}

TEST_F(SnakeLogicTest, move_down) {
  using enum SnakeLogic::Field;
  userInput(UserAction_t::Start, false);

  setSnakeToCenterFieldHeadUp();
  userInput(UserAction_t::Down, false);
  EXPECT_EQ(gameInfo.field[8][4], static_cast<int>(HEAD_UP));

  setSnakeToCenterFieldHeadDown();
  userInput(UserAction_t::Down, false);
  EXPECT_EQ(gameInfo.field[12][4], static_cast<int>(HEAD_DOWN));

  setSnakeToCenterFieldHeadLeft();
  userInput(UserAction_t::Down, false);
  EXPECT_EQ(gameInfo.field[10][3], static_cast<int>(HEAD_DOWN));

  setSnakeToCenterFieldHeadRight();
  userInput(UserAction_t::Down, false);
  EXPECT_EQ(gameInfo.field[10][6], static_cast<int>(HEAD_DOWN));
}

TEST_F(SnakeLogicTest, action) {
  using enum SnakeLogic::Field;
  userInput(UserAction_t::Start, false);

  setSnakeToCenterFieldHeadUp();
  userInput(UserAction_t::Action, false);
  EXPECT_EQ(gameInfo.field[7][4], static_cast<int>(HEAD_UP));

  setSnakeToCenterFieldHeadDown();
  userInput(UserAction_t::Action, false);
  EXPECT_EQ(gameInfo.field[12][4], static_cast<int>(HEAD_DOWN));

  setSnakeToCenterFieldHeadLeft();
  userInput(UserAction_t::Action, false);
  EXPECT_EQ(gameInfo.field[9][2], static_cast<int>(HEAD_LEFT));

  setSnakeToCenterFieldHeadRight();
  userInput(UserAction_t::Action, false);
  EXPECT_EQ(gameInfo.field[9][7], static_cast<int>(HEAD_RIGHT));

  for (int y = 0; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      gameInfo.field[y][x] = 0;
    }
  }

  gameInfo.field[0][0] = 8;
  gameInfo.field[1][0] = 7;
  gameInfo.field[1][1] = 6;
  gameInfo.field[0][1] = static_cast<int>(HEAD_LEFT);
  userInput(UserAction_t::Action, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::GAME_OVER);
}

TEST_F(SnakeLogicTest, win) {
  userInput(UserAction_t::Start, false);

  // fill field snake body
  int bodyId = 6;
  for (int y = 0; y < FIELD_HEIGHT; ++y) {
    if (y % 2 == 0) {  // body direct right
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        if (y == 0 && x < 3) continue;
        gameInfo.field[y][x] = bodyId++;
      }
    } else {  // body direct left
      for (int x = FIELD_WIDTH - 1; x >= 0; --x) {
        gameInfo.field[y][x] = bodyId++;
      }
    }
  }

  // set snake
  gameInfo.field[0][0] = 0;
  gameInfo.field[0][1] = 1;
  gameInfo.field[0][2] = static_cast<int>(SnakeLogic::Field::HEAD_LEFT);

  gameTick();
  gameTick();

  EXPECT_EQ(getCurrentGameStatus(), GameStatus::WIN);
  EXPECT_EQ(updateCurrentState().pause, 1);

  userInput(UserAction_t::Terminate, false);
  EXPECT_EQ(getCurrentGameStatus(), GameStatus::INIT);
}

TEST_F(SnakeLogicTest, win_2) {
  userInput(UserAction_t::Start, false);

  // fill field snake body
  int bodyId = 6;
  for (int y = 0; y < FIELD_HEIGHT; ++y) {
    if (y % 2 == 0) {  // body direct right
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        if (y == 0 && x < 3) continue;
        gameInfo.field[y][x] = bodyId++;
      }
    } else {  // body direct left
      for (int x = FIELD_WIDTH - 1; x >= 0; --x) {
        gameInfo.field[y][x] = bodyId++;
      }
    }
  }

  // set snake
  gameInfo.field[0][0] = 0;
  gameInfo.field[0][1] = 1;
  gameInfo.field[0][2] = static_cast<int>(SnakeLogic::Field::HEAD_LEFT);

  userInput(UserAction_t::Left, false);
  userInput(UserAction_t::Left, false);

  EXPECT_EQ(getCurrentGameStatus(), GameStatus::WIN);
  EXPECT_EQ(updateCurrentState().pause, 1);
}
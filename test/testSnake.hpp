#ifndef TEST_SNAKE_HPP
#define TEST_SNAKE_HPP

#include <gtest/gtest.h>

#include "../retro_games/snake/snakeLogic.hpp"

namespace s21 {

class SnakeLogicTest : public ::testing::Test, public SnakeLogic {
 protected:
  void fillUpperFieldFood() {
    for (int y = 0; y < FIELD_HEIGHT; ++y) {
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        gameInfo.field[y][x] = y == 0 ? 1 : 0;
      }
    }

    // set snake
    gameInfo.field[0][0] = 8;
    gameInfo.field[0][1] = 7;
    gameInfo.field[0][2] = 6;
    gameInfo.field[0][3] = static_cast<int>(SnakeLogic::Field::HEAD_RIGHT);
  }

  void setSnakeToCenterFieldHeadUp() {
    for (int y = 0; y < FIELD_HEIGHT; ++y) {
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        gameInfo.field[y][x] = 0;
      }
    }

    // set snake to center
    gameInfo.field[8][4] = static_cast<int>(SnakeLogic::Field::HEAD_UP);
    gameInfo.field[9][4] = 6;
    gameInfo.field[10][4] = 7;
    gameInfo.field[11][4] = 8;
  }

  void setSnakeToCenterFieldHeadDown() {
    for (int y = 0; y < FIELD_HEIGHT; ++y) {
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        gameInfo.field[y][x] = 0;
      }
    }

    // set snake to center
    gameInfo.field[8][4] = 8;
    gameInfo.field[9][4] = 7;
    gameInfo.field[10][4] = 6;
    gameInfo.field[11][4] = static_cast<int>(SnakeLogic::Field::HEAD_DOWN);
  }

  void setSnakeToCenterFieldHeadLeft() {
    for (int y = 0; y < FIELD_HEIGHT; ++y) {
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        gameInfo.field[y][x] = 0;
      }
    }

    // set snake to center
    gameInfo.field[9][3] = static_cast<int>(SnakeLogic::Field::HEAD_LEFT);
    gameInfo.field[9][4] = 6;
    gameInfo.field[9][5] = 7;
    gameInfo.field[9][6] = 8;
  }

  void setSnakeToCenterFieldHeadRight() {
    for (int y = 0; y < FIELD_HEIGHT; ++y) {
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        gameInfo.field[y][x] = 0;
      }
    }

    // set snake to center
    gameInfo.field[9][3] = 8;
    gameInfo.field[9][4] = 7;
    gameInfo.field[9][5] = 6;
    gameInfo.field[9][6] = static_cast<int>(SnakeLogic::Field::HEAD_RIGHT);
  }
};

}  // namespace s21

#endif  // TEST_SNAKE_HPP
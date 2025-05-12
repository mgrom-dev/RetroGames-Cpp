#ifndef TEST_TETRIS_HPP
#define TEST_TETRIS_HPP

#include <gtest/gtest.h>

#include "../retro_games/tetris/tetrisLogic.hpp"

namespace s21 {

class TetrisLogicTest : public ::testing::Test, public TetrisLogic {};

}  // namespace s21

#endif  // TEST_TETRIS_HPP
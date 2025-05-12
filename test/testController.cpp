#include "testController.hpp"

#include <thread>

using namespace s21;
using namespace std::chrono;

TEST_F(GameControllerTest, constructor) {
  auto view = std::make_unique<MockGameView>();
  view->setCurrentGameType(GameType::NONE);
  GameController controller(std::move(view));
  EXPECT_NO_THROW({ controller.run(); });
}

TEST_F(GameControllerTest, constructor_2) {
  GameInfo_t a;
  a.score = 100;
  a.field = new int*[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; i++) {
    a.field[i] = new int[FIELD_WIDTH]();
  }

  GameInfo_t b;

  b = a;

  EXPECT_EQ(b.score, a.score);
  EXPECT_TRUE(b == a);
}

TEST_F(GameControllerTest, constructor_3) {
  GameInfo_t a;
  GameInfo_t b = a;

  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST_F(GameControllerTest, constructor_4) {
  GameInfo_t a;
  GameInfo_t b;
  b.level = 10;

  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
}

TEST_F(GameControllerTest, constructor_5) {
  GameInfo_t original;
  original.score = 100;
  original.level = 2;
  GameInfo_t expected = original;

  GameInfo_t moved(std::move(original));
  EXPECT_EQ(moved.score, expected.score);
  EXPECT_EQ(moved.level, expected.level);
}

TEST_F(GameControllerTest, constructor_6) {
  GameInfo_t a;
  a.field = new int*[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; i++) {
    a.field[i] = new int[FIELD_WIDTH]();
  }
  GameInfo_t b = a;

  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST_F(GameControllerTest, constructor_7) {
  GameInfo_t a;
  a.next = new int*[NEXT_HEIGHT];
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    a.next[i] = new int[NEXT_WIDTH]();
  }
  GameInfo_t b;
  b.next = new int*[NEXT_HEIGHT];
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    b.next[i] = new int[NEXT_WIDTH]();
  }
  b.field = new int*[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; i++) {
    b.field[i] = new int[FIELD_WIDTH]();
  }
  a = b;

  EXPECT_TRUE(a == b);

  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      a.field[y][x] = 1;
    }
  }

  for (int y = 0; y < NEXT_HEIGHT; y++) {
    for (int x = 0; x < NEXT_WIDTH; x++) {
      a.next[y][x] = 1;
    }
  }

  EXPECT_TRUE(a != b);
}

TEST_F(GameControllerTest, constructor_8) {
  GameInfo_t a;
  GameInfo_t b;
  b.next = new int*[NEXT_HEIGHT];
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    b.next[i] = new int[NEXT_WIDTH]();
  }
  a = b;

  EXPECT_TRUE(a == b);

  a.field = new int*[FIELD_HEIGHT];
  for (int i = 0; i < FIELD_HEIGHT; i++) {
    a.field[i] = new int[FIELD_WIDTH]();
  }

  EXPECT_TRUE(a != b);
}

TEST_F(GameControllerTest, constructor_9) {
  GameInfo_t a;
  GameInfo_t b;
  b.next = new int*[NEXT_HEIGHT];
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    b.next[i] = new int[NEXT_WIDTH]();
  }

  EXPECT_TRUE(a != b);
}

TEST_F(GameControllerTest, constructor_10) {
  GameInfo_t a;
  a.next = new int*[NEXT_HEIGHT];
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    a.next[i] = new int[NEXT_WIDTH]();
  }
  GameInfo_t b;
  b.next = new int*[NEXT_HEIGHT];
  for (int i = 0; i < NEXT_HEIGHT; i++) {
    b.next[i] = new int[NEXT_WIDTH](1);
  }

  EXPECT_TRUE(a != b);
}

TEST_F(GameControllerTest, gameType_NONE) {
  mockView->setCurrentGameType(GameType::NONE);
  run();
  EXPECT_EQ(this->gameType, GameType::NONE);
}

TEST_F(GameControllerTest, gameType_TETRIS) {
  mockView->setCurrentGameType(GameType::TETRIS);
  std::thread controllerThread([this]() { run(); });

  for (int i = 0; i < 100; ++i) {
    if (this->gameType == GameType::TETRIS) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  mockView->setCurrentGameType(GameType::NONE);
  closeGame();
  if (controllerThread.joinable()) {
    controllerThread.join();
  }
}

TEST_F(GameControllerTest, gameType_SNAKE) {
  mockView->setCurrentGameType(GameType::SNAKE);
  std::thread controllerThread([this]() { run(); });

  for (int i = 0; i < 100; ++i) {
    if (this->gameType == GameType::SNAKE) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  mockView->setCurrentGameType(GameType::NONE);
  closeGame();
  if (controllerThread.joinable()) {
    controllerThread.join();
  }
}

TEST_F(GameControllerTest, sendInput) {
  mockView->setCurrentGameType(GameType::SNAKE);
  std::thread controllerThread([this]() { run(); });

  for (int i = 0; i < 100; ++i) {
    if (this->gameType == GameType::SNAKE) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  mockView->setCurrentGameType(GameType::NONE);
  userInput(Key::UP, false);
  userInput(Key::ESC, false);
  userInput(Key::ESC, false);
  if (controllerThread.joinable()) {
    controllerThread.join();
  }
}

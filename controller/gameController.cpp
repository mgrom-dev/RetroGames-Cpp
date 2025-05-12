#include "gameController.hpp"

#include <cmath>

using namespace s21;

static int getDelay(int level, int initialDelay = 1000) {
  const double k = pow(0.1, 1.0 / 9.0);  // reduction ratio
  return static_cast<int>(initialDelay * pow(k, level - 1));
}

GameController::GameController(std::unique_ptr<GameView> view)
    : view(std::move(view)) {}

void GameController::setView(std::unique_ptr<GameView> view) {
  this->view = std::move(view);
}

void GameController::run() {
  using namespace std::chrono;

  while (view) {
    gameType = view->selectGame();
    switch (gameType) {
      case GameType::TETRIS:
        model = std::make_unique<TetrisLogic>();
        break;
      case GameType::SNAKE:
        model = std::make_unique<SnakeLogic>();
        break;
      default:
        view.reset();
        break;
    }

    while (model && gameType != GameType::NONE) {
      auto currentTime = high_resolution_clock::now();
      auto diff = currentTime - model->lastTickTime;
      auto timeDiff = duration_cast<milliseconds>(diff).count();

      GameInfo_t gameInfo = model->updateCurrentState();
      if (timeDiff >= getDelay(gameInfo.speed)) {
        model->gameTick();
        gameInfo = model->updateCurrentState();
        view->render(gameInfo, model->getCurrentGameStatus(), gameType);
      }

      std::this_thread::sleep_for(milliseconds(10));
    }
  }
}

void GameController::userInput(Key key, bool hold) {
  struct Action {
    UserAction_t action;
    Key key;
  };
  const Action actions[] = {
      {UserAction_t::Action, Key::SPACE}, {UserAction_t::Terminate, Key::ESC},
      {UserAction_t::Action, Key::SPACE}, {UserAction_t::Left, Key::LEFT},
      {UserAction_t::Right, Key::RIGHT},  {UserAction_t::Down, Key::DOWN},
      {UserAction_t::Up, Key::UP},        {UserAction_t::Start, Key::ENTER},
      {UserAction_t::Pause, Key::P}};
  const int numActions = sizeof(actions) / sizeof(actions[0]);

  UserAction_t action = UserAction_t::Terminate;
  for (int i = 0; i < numActions; i++) {
    if (key == actions[i].key) {
      action = actions[i].action;
    }
  }

  if (model && gameType != GameType::NONE) {
    GameStatus gameStatus = model->getCurrentGameStatus();
    if (gameStatus == GameStatus::INIT && key == Key::ESC) {
      closeGame();
    } else {
      this->model->userInput(action, hold);
      GameInfo_t gameInfo = model->updateCurrentState();
      view->render(gameInfo, gameStatus, gameType);
    }
  }
}

void GameController::closeGame() {
  gameType = GameType::NONE;
  model.reset();
}
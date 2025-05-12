#ifndef GAME_SELECTOR_HPP
#define GAME_SELECTOR_HPP

#include <QApplication>
#include <QCloseEvent>
#include <QScreen>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

#include "../../controller/common.hpp"

namespace s21 {
class GameSelector : public QDialog {
 public:
  GameSelector(QWidget *parent = nullptr)
      : QDialog(parent), gameType(GameType::NONE) {
    setWindowTitle("Select the game:");

    QLabel *label = new QLabel("Select the game", this);
    label->setAlignment(Qt::AlignCenter);

    QPushButton *tetrisButton = new QPushButton("Tetris", this);
    QPushButton *snakeButton = new QPushButton("Snake", this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(label);
    layout->addWidget(tetrisButton);
    layout->addWidget(snakeButton);

    connect(tetrisButton, &QPushButton::clicked, this,
            &GameSelector::setTetris);
    connect(snakeButton, &QPushButton::clicked, this, &GameSelector::setSnake);

    setLayout(layout);
    adjustSize();
    setFixedSize(size());

    // align the window to the center of the screen
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
      QRect screenRect = screen->geometry();
      QSize windowSize = size();
      int x = (screenRect.width() - windowSize.width()) / 2;
      int y = (screenRect.height() - windowSize.height()) / 2;
      move(x, y);
    }
  }

  GameType getGameType() const { return gameType; }

 private slots:
  void setTetris() {
    gameType = GameType::TETRIS;
    accept();
  }
  void setSnake() {
    gameType = GameType::SNAKE;
    accept();
  }

 protected:
  void closeEvent(QCloseEvent *event) override {
    if (event) {
      QApplication::quit();
    }
  }

  void keyPressEvent(QKeyEvent *event) override {
    if (event->key() == Qt::Key_Escape) {
      close();
    } else {
      QWidget::keyPressEvent(event);
    }
  }

 private:
  GameType gameType;
};
}  // namespace s21

#endif  // GAME_SELECTOR_HPP

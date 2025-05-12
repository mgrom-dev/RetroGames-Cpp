#ifndef GAME_WINDOW_HPP
#define GAME_WINDOW_HPP

#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

#include "../gameView.hpp"
#include "gameField.hpp"

namespace s21 {
class DesktopView;

class GameWindow : public QWidget {
  Q_OBJECT

 public:
  explicit GameWindow(DesktopView* view, QWidget* parent = nullptr);

  void setColors(const QVector<QColor>& colors);
  void setColorsNext(const QVector<QColor>& colors);
  void setScore(int score);
  void setLevel(int level);
  void setSpeed(int speed);
  void setHighScore(int highScore);
  void setVisiblity(bool isVisible);
  void setGameField(int** field);
  void setNextField(int** field);
  void setTitle(const char* title);
  void showInfoMessage(const char* message);
  void hideInfoMessage();
  void hideNextField();

 signals:
  void scoreChanged(int);
  void levelChanged(int);
  void speedChanged(int);
  void highScoreChanged(int);
  void visibleChanged(bool);
  void gameFieldChanged(const std::vector<int>&);
  void nextFieldChanged(const std::vector<int>&);
  void windowTitleChanged(const char*);
  void infoMessageChanged(const char*);
  void infoMessageHidding();
  void nextFieldHidding();

 public slots:
  void updateScore(int score);
  void updateLevel(int level);
  void updateSpeed(int speed);
  void updateHighScore(int highScore);
  void updateVisible(bool isVisible);
  void updateGameField(const std::vector<int>& fieldData);
  void updateNextField(const std::vector<int>& fieldData);
  void updateWindowTitle(const char* title);
  void updateInfoMessage(const char* message);
  void infoMessageHide();
  void nextFieldHide();

 protected:
  void keyPressEvent(QKeyEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

 private:
  DesktopView* view;
  GameField* gameField;
  GameField* nextField;
  QWidget* statsWidget;
  QLabel* nextLabel;
  QLabel* scoreLabel;
  QLabel* levelLabel;
  QLabel* speedLabel;
  QLabel* highScoreLabel;
  QLabel* infoOverlayLabel;
};
}  // namespace s21

#endif  // GAME_WINDOW_HPP
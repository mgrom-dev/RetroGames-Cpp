#ifndef STATS_WIDGET_HPP
#define STATS_WIDGET_HPP

#include <QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

namespace s21 {
class GameStatsWidget : public QWidget {
 public:
  GameStatsWidget(QWidget* parent = nullptr) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);

    scoreLabel = new QLabel("Score: 0", this);
    levelLabel = new QLabel("Level: 1", this);
    speedLabel = new QLabel("Speed: 1", this);
    highScoreLabel = new QLabel("High Score: 0", this);

    layout->addWidget(scoreLabel);
    layout->addWidget(levelLabel);
    layout->addWidget(speedLabel);
    layout->addWidget(highScoreLabel);

    setLayout(layout);
  }

  void setScore(int score) {
    scoreLabel->setText(QString("Score: %1").arg(score));
  }

  void setLevel(int level) {
    levelLabel->setText(QString("Level: %1").arg(level));
  }

  void setSpeed(double speed) {
    speedLabel->setText(QString("Speed: %1x").arg(speed));
  }

  void setHighScore(int maxScore) {
    highScoreLabel->setText(QString("Max Score: %1").arg(maxScore));
  }

 private:
  QLabel* scoreLabel;
  QLabel* levelLabel;
  QLabel* speedLabel;
  QLabel* highScoreLabel;
};
}  // namespace s21

#endif  // STATS_WIDGET_HPP
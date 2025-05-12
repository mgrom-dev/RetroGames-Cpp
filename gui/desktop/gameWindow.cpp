#include "gameWindow.hpp"

#include "desktopView.hpp"

using namespace s21;

static void setupConnections(GameWindow* gw) {
#define GW(sig) &GameWindow::sig
  gw->connect(gw, GW(scoreChanged), gw, GW(updateScore));
  gw->connect(gw, GW(levelChanged), gw, GW(updateLevel));
  gw->connect(gw, GW(speedChanged), gw, GW(updateSpeed));
  gw->connect(gw, GW(highScoreChanged), gw, GW(updateHighScore));
  gw->connect(gw, GW(visibleChanged), gw, GW(updateVisible));
  gw->connect(gw, GW(gameFieldChanged), gw, GW(updateGameField));
  gw->connect(gw, GW(nextFieldChanged), gw, GW(updateNextField));
  gw->connect(gw, GW(windowTitleChanged), gw, GW(updateWindowTitle));
  gw->connect(gw, GW(infoMessageChanged), gw, GW(updateInfoMessage));
  gw->connect(gw, GW(infoMessageHidding), gw, GW(infoMessageHide));
  gw->connect(gw, GW(nextFieldHidding), gw, GW(nextFieldHide));
#undef GW
}

GameWindow::GameWindow(DesktopView* view, QWidget* parent)
    : QWidget(parent), view(view) {
  qRegisterMetaType<std::vector<int>>("std::vector<int>");
  setWindowTitle("Game Window");

  gameField = new GameField(FIELD_WIDTH, FIELD_HEIGHT, this);
  nextField = new GameField(NEXT_WIDTH, NEXT_HEIGHT, this);
  nextField->setVisible(false);

  statsWidget = new QWidget(this);
  QVBoxLayout* statsLayout = new QVBoxLayout(statsWidget);

  nextLabel = new QLabel("   NEXT:", statsWidget);
  nextLabel->setVisible(false);
  scoreLabel = new QLabel("Score: 0", statsWidget);
  levelLabel = new QLabel("Level: 1", statsWidget);
  speedLabel = new QLabel("Speed: 1", statsWidget);
  highScoreLabel = new QLabel("High Score: 0", statsWidget);

  statsLayout->addWidget(nextLabel);
  statsLayout->addWidget(nextField);
  statsLayout->addWidget(levelLabel);
  statsLayout->addWidget(speedLabel);
  statsLayout->addWidget(scoreLabel);
  statsLayout->addWidget(highScoreLabel);

  statsWidget->setLayout(statsLayout);

  QHBoxLayout* mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(gameField);
  mainLayout->addWidget(statsWidget);

  setLayout(mainLayout);
  adjustSize();
  setFixedSize(size());
  setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

  infoOverlayLabel = new QLabel(this);
  infoOverlayLabel->setAlignment(Qt::AlignCenter);
  infoOverlayLabel->setStyleSheet(
      "background-color: rgba(0, 0, 0, 153);"  // 153/255 ≈ 60% transparency
      "color: white;"
      "font-size: 24px;"
      "padding: 20px;"
      "border-radius: 10px;");
  infoOverlayLabel->setVisible(false);

  // Label on full window, no blocks mouse events
  infoOverlayLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
  infoOverlayLabel->setGeometry(this->rect());

  // align the window to the center of the screen
  QScreen* screen = QGuiApplication::primaryScreen();
  if (screen) {
    QRect screenRect = screen->geometry();
    QSize windowSize = size();
    int x = (screenRect.width() - windowSize.width()) / 2;
    int y = (screenRect.height() - windowSize.height()) / 2;
    move(x, y);
  }

  setFocusPolicy(Qt::StrongFocus);
  setupConnections(this);
}

void GameWindow::closeEvent(QCloseEvent* event) {
  view->gameWindowClosed();
  event->accept();
}

void GameWindow::setColors(const QVector<QColor>& colors) {
  gameField->setColors(colors);
}

void GameWindow::setColorsNext(const QVector<QColor>& colors) {
  nextField->setColors(colors);
}

void GameWindow::setScore(int score) { emit scoreChanged(score); }

void GameWindow::setLevel(int level) { emit levelChanged(level); }

void GameWindow::setSpeed(int speed) { emit speedChanged(speed); }

void GameWindow::setHighScore(int highScore) {
  emit highScoreChanged(highScore);
}

void GameWindow::setVisiblity(bool isVisible) {
  emit visibleChanged(isVisible);
}

void GameWindow::setGameField(int** field) {
  const int width = FIELD_WIDTH;
  const int height = FIELD_HEIGHT;

  // Copying a two-dimensional array into a one-dimensional vector (row-major)
  std::vector<int> fieldData(width * height);

  for (int y = 0; y < height; ++y) {
    memcpy(&fieldData[y * width], field[y], width * sizeof(int));
  }

  emit gameFieldChanged(fieldData);
}

void GameWindow::setNextField(int** field) {
  const int width = NEXT_WIDTH;
  const int height = NEXT_HEIGHT;

  // 1. search borders figure
  int maxX = -1;
  int maxY = -1;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      if (field[y][x] != 0) {
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
      }
    }
  }

  // if figure is empty
  if (maxX == -1 || maxY == -1) {
    emit nextFieldChanged(std::vector<int>(width * height, 0));
  } else {
    // 2. calc offset for centering
    int offsetX = (width - maxX - 1) / 2;
    int offsetY = (height - maxY - 1) / 2;

    // 3. create centering array
    std::vector<int> fieldData(width * height, 0);
    for (int srcY = 0, y = offsetY; y < height; ++y, ++srcY) {
      for (int srcX = 0, x = offsetX; x < width; ++x, ++srcX) {
        fieldData[y * width + x] = field[srcY][srcX];
      }
    }
    emit nextFieldChanged(fieldData);
  }
}

void GameWindow::setTitle(const char* title) { emit windowTitleChanged(title); }

void GameWindow::showInfoMessage(const char* message) {
  emit infoMessageChanged(message);
}

void GameWindow::hideInfoMessage() { emit infoMessageHidding(); }

void GameWindow::hideNextField() { emit nextFieldHidding(); }

void GameWindow::updateScore(int score) {
  scoreLabel->setText(QString("Score: %1").arg(score));
}

void GameWindow::updateLevel(int level) {
  levelLabel->setText(QString("Level: %1").arg(level));
}

void GameWindow::updateSpeed(int speed) {
  speedLabel->setText(QString("Speed: %1").arg(speed));
}

void GameWindow::updateHighScore(int highScore) {
  highScoreLabel->setText(QString("High Score: %1").arg(highScore));
}

void GameWindow::updateVisible(bool isVisible) {
  if (isVisible && !this->isVisible()) {
    show();
  } else if (!isVisible && this->isVisible()) {
    hide();
  }
}

void GameWindow::updateGameField(const std::vector<int>& fieldData) {
  gameField->updateField(fieldData);
}

void GameWindow::updateNextField(const std::vector<int>& fieldData) {
  nextField->setVisible(true);
  nextLabel->setVisible(true);
  nextField->updateField(fieldData);
}

void GameWindow::updateWindowTitle(const char* title) {
  setWindowTitle(QString::fromUtf8(title));
}

void GameWindow::updateInfoMessage(const char* message) {
  infoOverlayLabel->setText(QString::fromUtf8(message));
  infoOverlayLabel->setVisible(true);
  infoOverlayLabel->raise();
}

void GameWindow::infoMessageHide() { infoOverlayLabel->setVisible(false); }

void GameWindow::nextFieldHide() {
  nextField->setVisible(false);
  nextLabel->setVisible(false);
}

void GameWindow::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Escape:
      view->keyPressEvent(Key::ESC);
      break;
    case Qt::Key_P:
    case 1047:
      view->keyPressEvent(Key::P);
      break;
    case Qt::Key_Space:
      view->keyPressEvent(Key::SPACE);
      break;
    case Qt::Key_Enter:
    case 16777220:
      view->keyPressEvent(Key::ENTER);
      break;
    case Qt::Key_Left:
      view->keyPressEvent(Key::LEFT);
      break;
    case Qt::Key_Right:
      view->keyPressEvent(Key::RIGHT);
      break;
    case Qt::Key_Down:
      view->keyPressEvent(Key::DOWN);
      break;
    case Qt::Key_Up:
      view->keyPressEvent(Key::UP);
      break;
    default:
      QWidget::keyPressEvent(event);
      break;
  }
}
#ifndef GAME_FIELD_HPP
#define GAME_FIELD_HPP

#include <QColor>
#include <QPainter>
#include <QVector>
#include <QtWidgets/QFrame>

namespace s21 {
#define PIXEL_SIZE 20

class GameField : public QFrame {
 public:
  explicit GameField(int width, int height, QWidget* parent = nullptr, const QVector<QColor>& colors = QVector<QColor>())
      : QFrame(parent), width(width), height(height),
      colors(colors.empty() ? 
             QVector<QColor>{  // Дефолтные цвета если пустой массив
                 QColor(0, 0, 0),    // 0 - background
                 QColor(0, 0, 238) //  blue
             } : colors) {
    setFixedSize(width * PIXEL_SIZE, height * PIXEL_SIZE);
  }

  void updateField(const std::vector<int>& fieldData) {
    field = fieldData;
    update();
  }

  void setColors(const QVector<QColor>& colors) { this->colors = colors; }

 protected:
  void paintEvent(QPaintEvent* event) override {
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.fillRect(rect(), colors[0]);  // fill default background

    // draw field
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        int idx = y * width + x;
        if (static_cast<size_t>(idx) >= field.size()) continue;

        int colorId = field[idx];
        if (colorId < 0) colorId = 0;
        if (colorId >= colors.size()) colorId = colors.size() - 1;

        painter.fillRect(x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE,
                         colors[colorId]);
      }
    }
  }

 private:
  int width = 0;
  int height = 0;
  QVector<QColor> colors;
  std::vector<int> field;
};
}  // namespace s21

#endif  // GAME_FIELD_HPP
#include "common.hpp"

using namespace s21;

bool GameInfo_t::operator!=(const GameInfo_t& rhs) const {
  bool notEqual = false;

  if (score != rhs.score || high_score != rhs.high_score ||
      level != rhs.level || speed != rhs.speed || pause != rhs.pause) {
    notEqual = true;
  }

  for (int i = 0; i < FIELD_HEIGHT && !notEqual; ++i) {
    if (field != nullptr && rhs.field != nullptr) {
      if (field[i] != nullptr && rhs.field[i] != nullptr) {
        for (int j = 0; j < FIELD_WIDTH && !notEqual; ++j) {
          if (field[i][j] != rhs.field[i][j]) {
            notEqual = true;
          }
        }
      }
    } else if (field != rhs.field) {  // nullptr check
      notEqual = true;
    }
  }

  const int NEXT_SIZE = 4;
  for (int i = 0; i < NEXT_SIZE && !notEqual; ++i) {
    if (next != nullptr && rhs.next != nullptr) {
      if (next[i] != nullptr && rhs.next[i] != nullptr) {
        for (int j = 0; j < NEXT_SIZE && !notEqual; ++j) {
          if (next[i][j] != rhs.next[i][j]) {
            notEqual = true;
          }
        }
      }
    } else if (next != rhs.next) {  // nullptr check
      notEqual = true;
    }
  }

  return notEqual;
}

bool GameInfo_t::operator==(const GameInfo_t& rhs) const {
  return !(*this != rhs);
}

GameInfo_t& GameInfo_t::operator=(const GameInfo_t& rhs) {
  const int NEXT_SIZE = 4;
  if (this != &rhs) {
    score = rhs.score;
    high_score = rhs.high_score;
    level = rhs.level;
    speed = rhs.speed;
    pause = rhs.pause;

    // free memory if alloc
    if (field != nullptr) {
      for (int i = 0; i < FIELD_HEIGHT; ++i) {
        delete[] field[i];
      }
      delete[] field;
    }
    if (next != nullptr) {
      for (int i = 0; i < NEXT_SIZE; ++i) {
        delete[] next[i];
      }
      delete[] next;
    }

    // alloc and copy memory
    if (rhs.field == nullptr) {
      field = nullptr;
    } else {
      field = new int*[FIELD_HEIGHT];
      for (int i = 0; i < FIELD_HEIGHT; ++i) {
        field[i] = new int[FIELD_WIDTH];
        for (int j = 0; j < FIELD_WIDTH; ++j) {
          field[i][j] = rhs.field[i][j];
        }
      }
    }

    if (rhs.next == nullptr) {
      next = nullptr;
    } else {
      next = new int*[NEXT_SIZE];
      for (int i = 0; i < NEXT_SIZE; ++i) {
        next[i] = new int[NEXT_SIZE];
        for (int j = 0; j < NEXT_SIZE; ++j) {
          next[i][j] = rhs.next[i][j];
        }
      }
    }
  }

  return *this;
}

GameInfo_t::GameInfo_t(const GameInfo_t& rhs) {
  score = rhs.score;
  high_score = rhs.high_score;
  level = rhs.level;
  speed = rhs.speed;
  pause = rhs.pause;

  if (rhs.field != nullptr) {
    field = new int*[FIELD_HEIGHT];
    for (int i = 0; i < FIELD_HEIGHT; ++i) {
      field[i] = new int[FIELD_WIDTH];
      for (int j = 0; j < FIELD_WIDTH; ++j) {
        field[i][j] = rhs.field[i][j];
      }
    }
  } else {
    field = nullptr;
  }

  const int NEXT_SIZE = 4;
  if (rhs.next != nullptr) {
    next = new int*[NEXT_SIZE];
    for (int i = 0; i < NEXT_SIZE; ++i) {
      next[i] = new int[NEXT_SIZE];
      for (int j = 0; j < NEXT_SIZE; ++j) {
        next[i][j] = rhs.next[i][j];
      }
    }
  } else {
    next = nullptr;
  }
}

GameInfo_t::GameInfo_t(GameInfo_t&& rhs) noexcept
    : field(rhs.field),
      next(rhs.next),
      score(rhs.score),
      high_score(rhs.high_score),
      level(rhs.level),
      speed(rhs.speed),
      pause(rhs.pause) {
  rhs.field = nullptr;
  rhs.next = nullptr;
}

GameInfo_t::GameInfo_t() {
  field = nullptr;
  next = nullptr;
  score = 0;
  high_score = 0;
  level = 0;
  speed = 0;
  pause = 0;
}

GameInfo_t::~GameInfo_t() {
  const int NEXT_SIZE = 4;
  if (field != nullptr) {
    for (int i = 0; i < FIELD_HEIGHT; ++i) {
      delete[] field[i];
    }
    delete[] field;
    field = nullptr;
  }

  if (next != nullptr) {
    for (int i = 0; i < NEXT_SIZE; ++i) {
      delete[] next[i];
    }
    delete[] next;
    next = nullptr;
  }
}
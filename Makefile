GPP := g++ -Wall -Werror -Wextra -std=c++20
TEST_LIB := -lgtest -lpthread -lgtest_main
GCOV_FLAGS := -fprofile-arcs -ftest-coverage --coverage
TEST := test_retro_games
NAME := retro_games
TEST_CPP := test/testSnake.cpp test/testTetris.cpp test/testController.cpp

# check valgrind exist
VALGRIND_EXISTS := $(shell command -v valgrind > /dev/null 2>&1 && echo 1 || echo 0)
VALGRIND := valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --quiet -s
# check leaks exist
LEAKS_EXISTS := $(shell command -v leaks > /dev/null 2>&1 && echo 1 || echo 0)
LEAKS = leaks -atExit --
# check doxygen exist
DOXYGEN_EXISTS := $(shell command -v doxygen > /dev/null 2>&1 && echo 1 || echo 0)
# check qt exist
QT_EXISTS := $(shell command -v qmake > /dev/null 2>&1 && command -v moc > /dev/null 2>&1 && echo 1 || echo 0)
# check lcov exist
LCOV_EXISTS := $(shell command -v lcov > /dev/null 2>&1 && echo 1 || echo 0)
# check archiver exist
TAR_EXISTS := $(shell command -v tar > /dev/null 2>&1 && echo 1 || echo 0)

OBJECTS := retro_games/tetris/tetrisLogic.o retro_games/snake/snakeLogic.o controller/common.o controller/gameController.o
CONSOLE_SOURCES := gui/console/consoleView.cpp

DESKTOP_SOURCES := gui/desktop/desktopView.cpp gui/desktop/gameWindow.cpp
MOC_HPP := gui/desktop/gameWindow.hpp
MOC_SOURCES := gui/desktop/moc_gameWindow.cpp
# qt library
QT_LIB += -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets
QT_LIB += -I/usr/include/x86_64-linux-gnu/qt5
QT_LIB += -I/usr/include/x86_64-linux-gnu/qt5/QtCore
QT_LIB += -I/usr/include/x86_64-linux-gnu/qt5/QtGui
LIBGAME = libgame.a

SOURCES_CLANG := $(wildcard */*.cpp) $(wildcard */*.hpp) $(wildcard */*/*.cpp) $(wildcard */*/*.hpp)
#SOURCES_CLANG := $(shell find . -type f \( -iname "*.cpp" -o -iname "*.hpp" \))

all: clean console desktop

$(LIBGAME): $(OBJECTS)
	@ar rcs $@ $^ && echo "the library with the game logic has been compiled"

$(LIBGAME:.a=_coverage.a): $(OBJECTS:.o=_cov.o)
	@ar rcs $@ $^ && echo "the library with the game logic is compiled with code coverage"

console: $(LIBGAME) $(CONSOLE_SOURCES)
	@$(GPP) -o $(NAME)_console $(CONSOLE_SOURCES) -L. -lgame && echo "the program with the console interface has been successfully compiled"
	
desktop: $(LIBGAME) $(DESKTOP_SOURCES)
	@if [ $(QT_EXISTS) -eq 1 ]; then \
		moc $(MOC_HPP) -o $(MOC_SOURCES); \
		$(GPP) -o $(NAME)_desktop $(DESKTOP_SOURCES) $(MOC_SOURCES) -L. -lgame $(QT_LIB) -fPIC -Wno-deprecated-enum-enum-conversion -lQt5Widgets -lQt5Core -lQt5Gui && echo "the program with the desktop interface has been successfully compiled"; \
	else \
		echo "The qt framework is not installed for compiling a desktop application, try the command: make console"; \
		echo "Visit the website: https://www.qt.io to install the qt framework"; \
		echo "If you are using Linux, try install it: sudo apt install qt5-qmake qtbase5-dev qtbase5-dev-tools"; \
	fi;

install: all
	@mkdir bin
	@cp ./$(NAME)_console ./bin/
	@cp ./$(NAME)_desktop ./bin/ && echo "retro_games installed to directory bin."

uninstall: clean
	@rm -rf ./bin/ && echo "space has been cleared, brick_game has been deleted from bin."

dvi: clean
	@if [ $(DOXYGEN_EXISTS) -eq 1 ]; then \
		doxygen Doxyfile; \
		xdg-open ./docs/html/index.html; \
	else \
		echo "fail make dvi, doxygen not installed..."; \
		echo "if you use linux try install it: sudo apt install doxygen"; \
	fi;

dist: clean
	@if [ $(TAR_EXISTS) -eq 1 ]; then \
		tar -czf retro_games.tar.gz retro_games controller gui Dockerfile Doxyfile Makefile retro_games-fsm.png && echo "archived distrubutive retro_games.tar.gz was successfully created"; \
	else \
		echo "The zip distribution could not be created, the tar archive was not found."; \
		echo "if you use linux try install it: sudo apt install tar"; \
	fi;

%.o: %.cpp
	@$(GPP) -c $< -o $@

%_cov.o: %.cpp
	@$(GPP) $(GCOV_FLAGS) -c $< -o $@

gcov_report: clean $(LIBGAME:.a=_coverage.a) $(TEST_CPP)
	@if [ $(LCOV_EXISTS) -eq 1 ]; then \
		$(GPP) $(GCOV_FLAGS) $(TEST_CPP) -o $(TEST) -L. -lgame_coverage $(TEST_LIB); \
		./$(TEST); \
		lcov -t "gcov_report" -o tests.info --no-external -c -d .; \
		genhtml -o tests tests.info; \
		xdg-open ./tests/index.html; \
	else \
		echo "the lcov utility for generating code coverage reports is not installed..."; \
		echo "if you use linux try install it: sudo apt install lcov"; \
	fi;

gcov_report_docker: clean $(LIBGAME:.a=_coverage.a) $(TEST_CPP)
	$(GPP) $(GCOV_FLAGS) $(TEST_CPP) -o $(TEST) -L. -lgame_coverage $(TEST_LIB)
	./$(TEST)
	lcov --capture --no-external --directory . --output-file tests.info --ignore-errors mismatch
	genhtml -o tests tests.info

test: clean compile_test
	@./$(TEST)

compile_test: $(LIBGAME) $(TEST_CPP)
	@$(GPP) -o $(TEST) $(TEST_CPP) -L. -lgame $(TEST_LIB) && echo "the gtests compiled"

leaks: clean compile_test
	@if [ $(VALGRIND_EXISTS) -eq 1 ]; then \
		$(VALGRIND) ./$(TEST); \
	elif [ $(LEAKS_EXISTS) -eq 1 ]; then \
		$(LEAKS) ./$(TEST); \
	else \
		echo "You need to install the valgrind application (recommended) or leaks to check for leaks."; \
		echo "Visit the website: https://valgrind.org/"; \
		echo "if you use linux try install it: sudo apt install valgrind"; \
	fi;

docker: clean
	docker build -t alpine_gpp .
	docker run -d --name retro_games alpine_gpp sleep infinity
	docker cp ./ retro_games:/app/
	docker exec --workdir=//app retro_games make -i gcov_report_docker
	docker cp retro_games://app/tests ./
	docker rm -f retro_games
	open ./tests/index.html || powershell -Command "Start-Process ./tests/index.html"

style:
	clang-format -n -style=Google $(SOURCES_CLANG)

clang:
	clang-format -i -style=Google $(SOURCES_CLANG)

clean:
	@rm -rf *.o */*.o */*/*.o
	@rm -rf *.gcno */*.gcno */*/*.gcno
	@rm -rf *.gcda */*.gcda */*/*.gcda
	@rm -rf $(NAME)_console $(NAME)_desktop $(TEST) $(MOC_SOURCES) *.db *.a ./tests/ *.info *.gz ./docs/
	$(info the compiled files have been deleted, and the disk space has been freed)

.PHONY: all clean

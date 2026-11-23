CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude
SRC = src/Common.cpp src/Graph.cpp src/BusManager.cpp src/DataManager.cpp src/UI.cpp src/App.cpp src/main.cpp
TEST_SRC = src/Common.cpp src/Graph.cpp src/BusManager.cpp src/DataManager.cpp src/UI.cpp tests/test_algorithms.cpp
TARGET = TransitIQ.exe
TEST_TARGET = test_algorithms.exe

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) -o $(TEST_TARGET)

clean:
	del /f /q $(TARGET) $(TEST_TARGET) *.o 2>nul || rm -f $(TARGET) $(TEST_TARGET) *.o

.PHONY: all test clean


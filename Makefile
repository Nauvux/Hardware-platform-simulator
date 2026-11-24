CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iheader

SRC_DIR = programs
TEST_DIR = tests
OBJ_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
# Tout sauf main.o : partagé entre le simulateur et les tests
LIB_OBJS = $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

TARGET = simulator
TEST_TARGET = test_cpu

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# make test : compile puis lance les tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(LIB_OBJS) $(OBJ_DIR)/test_cpu.o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/test_cpu.o: $(TEST_DIR)/test_cpu.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_TARGET)

.PHONY: all test clean

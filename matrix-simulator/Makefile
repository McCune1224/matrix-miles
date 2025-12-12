CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O2 -I include
LDFLAGS = -lcurl

SRC_DIR = src
OBJ_DIR = build
TARGET = simulator

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean run compile_commands

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(TARGET)
	./$(TARGET)

# Generate compile_commands.json for clangd LSP
compile_commands: clean
	bear -- $(MAKE) all

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

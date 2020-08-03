BIN = main
CXX = clang++-10
CPP_FLAGS = -std=c++17 -Wall -Wextra -pedantic -MMD -MP
SRC_DIR = ./src

ifdef DEBUG
	CPP_FLAGS += -g
	BUILD_DIR = ./build/debug
else
	CPP_FLAGS += -O3
	BUILD_DIR = ./build/release
endif

$(BUILD_DIR)/$(BIN): $(BUILD_DIR)/main.o
	$(CXX) $< -o $@

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CPP_FLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean

-include $(BUILD_DIR)/main.d
MAKEFLAGS += -j3 # parallel processes
CXX = g++
CXX_FLAGS = -std=c++11 -Wall -O3 -march=native -funroll-loops -fstrict-aliasing -g -rdynamic

BIN = igr
SRC_DIR = ./src
BIN_DIR = ./bin
OUT_DIR = ./out

# Source files
CPP = $(wildcard $(SRC_DIR)/*.cpp)

# All .o files go to build dir
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OUT_DIR)/%.o, $(CPP))

# GCC will create these .d files containing dependencies
DEP = $(OBJ:%.o=%.d)

# Default target
$(BIN) : $(BIN_DIR)/$(BIN)

# Link binary
$(BIN_DIR)/$(BIN) : $(OBJ)
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $^ -o $@

# Include all .d files
-include $(DEP)

# Compile source to object (and generate dependency file)
$(OUT_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -rf -- $(OUT_DIR) $(BIN_DIR)/$(BIN)




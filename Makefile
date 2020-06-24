CXX = g++
CXXFLAGS = -Wall -g -Werror -std=c++14
LDFLAGS =

SRC = src
OBJ = obj
BIN = bin

SRCS = $(wildcard $(SRC)/*.cc)
OBJS = $(addprefix $(OBJ)/, $(patsubst %.cc,%.o,$(notdir $(SRCS))))
DEPS = $(OBJS:%.o=%.d)

TARGET_NAME = dukkha
TARGET = $(BIN)/$(TARGET_NAME)
TARGET_ARGS = examples/statement.du

$(TARGET): $(OBJS)
	$(CXX) $^ $(LDFLAGS) -o $(TARGET)

$(OBJ)/%.o: $(SRC)/%.cc
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

-include $(DEPS)

.PHONY: run
run: $(TARGET)
	./$(TARGET) $(TARGET_ARGS)

.PHONY: clean
clean:
	rm $(OBJ)/* $(BIN)/*

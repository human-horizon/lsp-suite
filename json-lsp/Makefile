CXX = zig c++
CXXFLAGS = -Wall -Wextra -O2 -g -std=c++17

SRC = src/main.cpp src/lsp.cpp src/json-parser.cpp src/simdjson.cpp
OBJ = $(SRC:.cpp=.o)
BIN = json-lsp

.PHONY: all clean install

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(BIN)

install: $(BIN)
	cp $(BIN) /usr/local/bin/

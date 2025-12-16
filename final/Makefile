CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = ejemplo_rtree
SOURCES = ejemplo_rtree.cpp
HEADERS = rtree.h rtreenode.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) rtree_data.bin

.PHONY: all run clean

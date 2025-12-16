CXX = g++
CXXFLAGS = -std=c++17 -Wall -g


TARGET = main
SRCS = main.cpp 

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-del /f /q $(OBJS) $(TARGET).exe

run: $(TARGET)
	./$(TARGET).exe

.PHONY: all clean
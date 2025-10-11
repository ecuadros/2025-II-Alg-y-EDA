CXX = g++

CXXFLAGS = -std=c++17 -Wall -g

TARGET = main

SRCS = main.cpp ContainersDemo.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
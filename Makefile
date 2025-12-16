CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -pthread # Añadido -pthread
LDFLAGS = -pthread # Añadido -pthread

TARGET = main
SRCS = main.cpp \
       hilos.cpp \
	   DemoVector.cpp \
	   DemoList.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

# R-Tree demo
demo_rtree: demo_rtree.o
	$(CXX) $(LDFLAGS) $^ -o $@

demo_rtree.o: demo_rtree.cpp rtree.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) demo_rtree.o $(TARGET) demo_rtree

.PHONY: all clean
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -pthread # Añadido -pthread
LDFLAGS = -pthread # Añadido -pthread

TARGET = main
SRCS = main.cpp \
       hilos.cpp \
	   DemoVector.cpp \
	   DemoList.cpp

OBJS = $(SRCS:.cpp=.o)

all: demo

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@

demo: demo.o
	$(CXX) $(LDFLAGS) $^ -o $@

demo.o: demo.cpp btree.h btreepage.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) demo.o $(TARGET) demo

.PHONY: all clean
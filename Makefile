CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

TARGET = lapifetch

SRC = main.cpp system.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

install: all
	cp $(TARGET) /usr/local/bin

uninstall:
	rm -f /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET)
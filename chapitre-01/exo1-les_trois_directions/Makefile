CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

TARGET = exo1

all: $(TARGET)

$(TARGET): main.cpp Directions.cpp
	$(CXX) $(CXXFLAGS) main.cpp Directions.cpp -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

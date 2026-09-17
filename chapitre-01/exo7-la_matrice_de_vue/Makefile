CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LDLIBS = -lglfw3 -lopengl32 -lgdi32

TARGET = exo7

all: $(TARGET)

$(TARGET): main.cpp NkVec3f.hpp NkQuatf.hpp Pose.hpp Mat4.hpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET) $(TARGET).exe

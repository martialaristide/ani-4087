CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
LDLIBS = -lglfw3 -lopengl32 -lgdi32

TARGET = exo6
TESTS_TARGET = tests

HEADERS = NkVec3f.hpp NkQuatf.hpp Pose.hpp Composer.hpp ArmModel.hpp ReferenceKinematics.hpp

all: $(TARGET)

$(TARGET): main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET) $(LDLIBS)

test: $(TESTS_TARGET)
	./$(TESTS_TARGET)

$(TESTS_TARGET): tests.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) tests.cpp -o $(TESTS_TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe $(TESTS_TARGET) $(TESTS_TARGET).exe

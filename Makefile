# Makefile for AntiDetection project

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -luser32

# Target executable
TARGET = antidetection_demo

# Source files
SOURCES = main.cpp AntiDetection.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = AntiDetection.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Rebuild from scratch
rebuild: clean all

# Run the demo
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run
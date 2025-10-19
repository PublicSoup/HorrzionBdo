# Makefile for Black Desert Online Anti-Detection System

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Platform detection
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    LDFLAGS = -lstdc++fs -pthread
else
    LDFLAGS = -luser32 -lstdc++fs
endif

# Target executable
TARGET = bdo_antidetection

# Source files
SOURCES = main.cpp AntiDetection.cpp BDOAntiDetection.cpp BDOConfig.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = AntiDetection.h BDOAntiDetection.h BDOConfig.h

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
	rm -rf profiles/

# Rebuild from scratch
rebuild: clean all

# Run the demo
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build with maximum optimization
release: CXXFLAGS = -std=c++17 -O3 -march=native -DNDEBUG
release: clean $(TARGET)

# Install (copy to system location)
install: $(TARGET)
	@echo "Installing BDO Anti-Detection System..."
	@mkdir -p /usr/local/bin
	@cp $(TARGET) /usr/local/bin/
	@echo "Installation complete. Run 'bdo_antidetection' from anywhere."

# Create distribution package
dist: release
	@echo "Creating distribution package..."
	@mkdir -p dist
	@cp $(TARGET) dist/
	@cp *.h dist/
	@cp *.cpp dist/
	@cp Makefile dist/
	@cp README.md dist/
	@tar -czf bdo_antidetection_dist.tar.gz dist/
	@rm -rf dist/
	@echo "Distribution package created: bdo_antidetection_dist.tar.gz"

.PHONY: all clean rebuild run debug release install dist
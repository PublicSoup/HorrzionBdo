# Makefile for BDO Anti-Cheat Research Framework

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -luser32 -lpsapi -ldbghelp -ladvapi32

# Target executables
TARGETS = antidetection_demo bdo_research

# Common headers
HEADERS = AntiDetection.h BDOAntiDetection.h BDOAntiCheatResearch.h

# AntiDetection demo sources
DEMO_SOURCES = main.cpp AntiDetection.cpp
DEMO_OBJECTS = $(DEMO_SOURCES:.cpp=.o)

# BDO Research sources
RESEARCH_SOURCES = BDOResearchDemo.cpp BDOAntiCheatResearch.cpp AntiDetection.cpp
RESEARCH_OBJECTS = $(RESEARCH_SOURCES:.cpp=.o)

# Default target - build all
all: $(TARGETS)

# Build AntiDetection demo
antidetection_demo: $(DEMO_OBJECTS)
	$(CXX) $(DEMO_OBJECTS) -o $@ $(LDFLAGS)

# Build BDO Research tool
bdo_research: $(RESEARCH_OBJECTS)
	$(CXX) $(RESEARCH_OBJECTS) -o $@ $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f *.o $(TARGETS) *.exe

# Build only the research tool
research: bdo_research

# Build only the demo
demo: antidetection_demo

# Run the research tool
run-research: bdo_research
	./bdo_research

# Run the demo
run-demo: antidetection_demo
	./antidetection_demo

# Rebuild everything
rebuild: clean all

# Install (Windows - copy to appropriate directory)
install:
	@echo "Installing BDO Research Framework..."
	@echo "Run as administrator for system-wide installation"

.PHONY: all clean rebuild research demo install run-research run-demo
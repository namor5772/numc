# Builds the numc solver on macOS/Linux. On Windows use numc.sln (see README.md).
CXX ?= c++
CXXFLAGS ?= -O2 -Wall

numc: numc.cpp
	$(CXX) $(CXXFLAGS) -o $@ numc.cpp

.PHONY: clean
clean:
	rm -f numc

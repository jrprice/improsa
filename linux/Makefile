EXE=improsa
SRCDIR=../src
OBJDIR=obj

SUFFIXES += .d

CXX      = g++
CXXFLAGS = -I$(SRCDIR) -std=c++11 -O2
LDFLAGS  = -lOpenCL -lpthread
MODULES  = Filter Blur Sharpen Sobel
OBJECTS  = $(MODULES:%=$(OBJDIR)/%.o)
SOURCES  = $(MODULES:%=$(SRCDIR)/%.cpp)
DEPFILES = $(MODULES:%=$(OBJDIR)/%.d)

all: prebuild $(OBJDIR) $(EXE)

$(EXE): $(OBJECTS) improsa.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) halide/*.s -o $(EXE) improsa.cpp

prebuild:
	$(MAKE) -C ../src/opencl -f $(shell pwd)/Makefile opencl
	$(MAKE) -C ../src/halide -f $(shell pwd)/Makefile halide

opencl:
	./stringify_kernels.sh

halide:
	HL_TARGET=x86-64-opencl ./gen_filters.sh ../../linux/halide

$(OBJDIR)/%.d: $(SRCDIR)/%.cpp $(OBJDIR)
	$(CXX) $(CXXFLAGS) -MM -MT $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$<) $< -MF $@ 2>/dev/null

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(EXE) ../src/opencl/*.h halide

.PHONY: clean

ifeq (0, $(words $(findstring $(MAKECMDGOALS), clean opencl halide)))
-include $(DEPFILES)
endif

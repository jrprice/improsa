# Makefile (ImProSA)
# Copyright (c) 2014, James Price and Simon McIntosh-Smith,
# University of Bristol. All rights reserved.
#
# This program is provided under a three-clause BSD license. For full
# license terms please see the LICENSE file distributed with this
# source code.

EXE=improsa
SRCDIR=../src
OBJDIR=obj

SUFFIXES += .d

CXX      = g++
CXXFLAGS = -I$(SRCDIR) -O2 -DCL_USE_DEPRECATED_OPENCL_1_1_APIS
LDFLAGS  = -lOpenCL -lpthread
MODULES  = Filter Bilateral Blur Sharpen Sobel
OBJECTS  = $(MODULES:%=$(OBJDIR)/%.o)
SOURCES  = $(MODULES:%=$(SRCDIR)/%.cpp)
DEPFILES = $(MODULES:%=$(OBJDIR)/%.d)

ifneq ($(wildcard .halide),)
	HALIDE = 1
endif
ifeq ($(HALIDE),1)
	CXXFLAGS += -DENABLE_HALIDE
	FILTERS = bilateral blur sharpen sobel
	HALIDE_FILES = $(FILTERS:%=halide/%_cpu.s)
	HALIDE_FILES += $(FILTERS:%=halide/%_gpu.s)
endif

all: prebuild $(OBJDIR) $(EXE)

halide:
	$(MAKE) all HALIDE=1

$(EXE): $(OBJECTS) $(HALIDE_FILES) improsa.cpp
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

prebuild:
	$(MAKE) -C ../src/opencl -f $(shell pwd)/Makefile prebuild_opencl
ifeq ($(HALIDE),1)
	$(MAKE) -C ../src/halide -f $(shell pwd)/Makefile prebuild_halide
endif

prebuild_opencl:
	./stringify_kernels.sh

prebuild_halide:
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

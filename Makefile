##########################
# MACRO
##########################

program := sim

sources := $(wildcard *.cpp)
objects := $(subst .cpp,.o,$(sources))

dram_library := ./DRAMsim3/libdramsim3.so
LDLIBS := -ldramsim3

CXXFLAGS += -std=c++11 -Wall -g
LDFLAGS += -L$(dir $(dram_library))
include_dir := -soname

CXX := g++
RM := rm -f

#########################
# rules
#########################

.PHONY: all
all: $(program)

$(program): $(dram_library) $(objects)
	$(LINK.cc) -o $@ $(objects) $(LDLIBS) -I$(dir $(dram_library)) -Wl,-rpath=$(dir $(dram_library))

$(dram_library):
	$(MAKE) --directory=$(dir $@) $(notdir $@)

%.o: %.cpp
	$(COMPILE.cpp) $< -I$(dir $(dram_library))

.PHONY: clean
clean:
	$(MAKE) --directory=$(dir $(dram_library)) clean
	$(RM) $(objects) $(program)
	$(RM) $(test_objects) $(test)


#############################
# 참고
#############################
# LINK.o   = $(CC) $(LDFLAGS) $(TARGET_ARCH)
# LINK.cc  = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(TARGET_ARCH)
# LINK.C   = $(LINK.cc)
# LINK.cpp = $(LINK.cc)

# Or for compiling:

# COMPILE.cc  = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
# COMPILE.C   = $(COMPILE.cc)
# COMPILE.cpp = $(COMPILE.cc)
-include ../Makefile.config

LLVM_CONFIG ?= llvm-config

.PHONY: all clean

PDF = test.png StopWatch.png main.png Camera/Main.png Keyboard.png

all: $(PDF)

clean:
	rm -f $(PDF)

CLANG++ ?= $(shell $(LLVM_CONFIG) --bindir)/clang++
ifeq ($(wildcard $(CLANG++)),)
CLANG++ = clang++
endif
VISUALIZER = -Xclang -load -Xclang ../src/visualizer.so -Xclang -plugin -Xclang visualize-statechart

%.o %.dot: %.cpp ../src/visualizer.so
	$(CLANG++) $(VISUALIZER) $(CXXFLAGS) -c -o $(<:.cpp=.o) $<

%.png: %.dot
	dot -Tpng $< > $@

../src/visualizer:
	$(MAKE) -C ../src

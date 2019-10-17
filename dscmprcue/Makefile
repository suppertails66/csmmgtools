IDIR := src
SRCDIR := src
ODIR := obj
LDIR :=

CXX=gcc
CDEFINES = 
CFLAGS = -O2
CINCLUDES = 
CXXFLAGS=$(CFLAGS) $(CDEFINES) $(CINCLUDES) -I$(IDIR)

TOOLSRCS := $(wildcard $(SRCDIR)/*.c)
TOOLSINDIR := $(patsubst %.c,%,$(TOOLSRCS))
TOOLS := $(notdir $(TOOLSINDIR))

.SECONDEXPANSION:

all: $(TOOLS)

$(TOOLS): $(SRCDIR)/$$@.c $(LIBDEPS)
	$(CXX) $(SRCDIR)/$@.c $(OBJ) -o $(notdir $@) $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f $(TOOLS)

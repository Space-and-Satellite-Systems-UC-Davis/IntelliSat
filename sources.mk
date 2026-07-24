# Auto-discover every .c file in the tree (paths relative, no leading ./).
# (':=' expands this once when the makefile is read, not on every reference. The
# find runs on every `make`, but for a tree this size the cost is negligible.)
ALL_SOURCES := $(shell find . -name '*.c' | sed 's|^\./||')

# Files that must NOT go into the library. New source files do NOT belong here —
# only add a file if it breaks the build for the reason noted on its line.
EXCLUDE :=

# NOVAS test driver — has its own main()
EXCLUDE += determination/novasc3.1/checkout-mp.c           
EXCLUDE += determination/novasc3.1/checkout-stars.c        
EXCLUDE += determination/novasc3.1/checkout-stars-full.c 

# NOVAS build-time tool — has its own main()
EXCLUDE += determination/novasc3.1/cio_file.c           

# NOVAS usage examples — has its own main()
EXCLUDE += determination/novasc3.1/example.c     

# SGP4 test harness — has its own main()
EXCLUDE += determination/pos_lookup/sgp4/src/c/TestSGP4.c

# unit tests and test stubs — not part of the library, built by the test target
EXCLUDE += tests/%

# standalone ground-station tool — has its own main()
EXCLUDE += groundstation/inrange.c                         
EXCLUDE += groundstation/pointdish.c

# alternate novas backend — redefines solarsystem(), clashes with solsys1.c
EXCLUDE += determination/novasc3.1/solsys2.c               
EXCLUDE += determination/novasc3.1/solsys3.c              

SOURCES := $(filter-out $(EXCLUDE),$(ALL_SOURCES))

OBJECTS := $(SOURCES:.c=.o)

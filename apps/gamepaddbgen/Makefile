# -----------------------------------------------------------------------------
# gamepaddbgen — offline tool that turns gamecontrollerdb.txt into a C blob.
#
# Usage:
#   make                         # build the binary
#   make run                     # build + generate the default header
#   make clean                   # remove the binary and build dir
#
# Override variables on the command line if needed:
#   make INPUT=path/to/db.txt OUTPUT=path/to/out.h SYMBOL=my_table_name
# -----------------------------------------------------------------------------

CC      ?= cc
CSTD    ?= -std=c99
CFLAGS  ?= $(CSTD) -O2 -Wall -Wextra -Wno-unused-parameter
LDFLAGS ?=
LDLIBS  ?= -lm

ROOT     := ../..
BUILD    := $(ROOT)/build/gamepaddbgen
TARGET   := $(BUILD)/gamepaddbgen
SRC      := gamepaddbgen.c

INCLUDES := -I$(ROOT) -I$(ROOT)/examples

# Defaults for `make run`. Override on the command line.
INPUT    ?= $(ROOT)/docs_fpl/gamecontrollerdb.txt
OUTPUT   ?= $(ROOT)/apps/data/fpl-game-controller-db.h
SYMBOL   ?= fpl__g_gamepadMappingTable

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC) | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD):
	mkdir -p $(BUILD)

run: $(TARGET)
	$(TARGET) $(INPUT) $(OUTPUT) $(SYMBOL)

clean:
	rm -rf $(BUILD)

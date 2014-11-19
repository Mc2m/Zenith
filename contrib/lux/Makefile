
CC       := gcc
LD       := $(CC)
AR       := ar -cvq
RM       := rm -rf

CFLAGS   += -c -Wall -D_DEBUG

WORKDIR  := $(shell pwd)
OBJSDIR  := .objs/

OUT      := lux
SOURCES  := $(shell find -type f -name '*.c' | cut -c 3-)
OBJECTS  := $(SOURCES:%.c=$(OBJSDIR)%.o)
PCH_FILE := stdafx.h
OBJSPCH  := $(PCH_FILE:%.h=$(OBJSDIR)%.h.gch)
CMF      := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

#all
all: $(OBJSPCH) $(OBJECTS)
	$(AR) lib$(OUT).a $(OBJECTS)

#cleaning
.PHONY: clean
clean:
	$(RM) $(OBJSDIR) lib$(OUT).a

#compiling the precompiled header
$(OBJSDIR)%.h.gch:%.h $(CMF)
	@dirname $@ | xargs mkdir -p 2>/dev/null || echo "$@ already exists" >/dev/null
	@echo "Precompiling header $@..."
	@$(CC) $(CFLAGS) -o $@ -c $< || echo "error. Disabling precompiled header"
	@echo "...Done"

#compiling source files
$(OBJSDIR)%.o: %.c $(CMF)
	@dirname $@ | xargs mkdir -p 2>/dev/null || echo "$@ already exists" >/dev/null
	$(CC) $(CFLAGS) -I$(WORKDIR) $(PCH_FILE:%.h=-include %.h) -c $< -o $@

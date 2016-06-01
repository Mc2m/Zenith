
CC       := gcc
LD       := $(CC)
AR       := ar -cvq
RM       := rm -rf

CFLAGS   += -g -c -Wall -D_DEBUG

WORKDIR  := $(shell pwd)
OBJSDIR  := .objs/
CTRBDIR  := ../contrib/

CONTRIBS := $(shell ls -d $(CTRBDIR)*/ | grep -v "autobuild")
CTRBSPTH := $(shell for i in ${CONTRIBS}; do if [ -d "$${i}src" ]; then echo "$${i}src"; else echo "$$i"; fi; done)


OUT      := zenith
SOURCES  := $(shell find -type f -name '*.c' -not -iwholename '*tests*' | cut -c 3-)
OBJECTS  := $(SOURCES:%.c=$(OBJSDIR)%.o)
PCH_FILE := stdafx.h
OBJSPCH  := $(PCH_FILE:%.h=$(OBJSDIR)%.h.gch)
CMF      := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

TSRC     := $(shell find -type f -name '*.c' -iwholename '*tests*' | cut -c 3-)
TOBJS    := $(TSRC:%.c=$(OBJSDIR)%.o)
TLIBS    := $(OUT) luajit dl lux terra pthread m
TOUT     := $(OUT)_test

#all
all: contrib lib $(TOBJS)
	$(LD) -o $(TOUT) $(TOBJS) -L$(WORKDIR) $(CTRBSPTH:%=-L%) $(TLIBS:%=-l%)
	cp tests/$(OUT)_test.lua .

#cleaning
.PHONY: clean
clean:
	$(RM) $(OBJSDIR) $(TOUT) lib$(OUT).a $(OUT)_test.lua
	
.PHONY: clean-contrib
clean-contrib:
	$(foreach var,$(CONTRIBS),$(MAKE) clean -C $(var);)

#library
.PHONY: lib
lib: $(OBJSPCH) $(OBJECTS)
	$(AR) lib$(OUT).a $(OBJECTS)
	
#compiling the contribs
.PHONY: contrib
contrib:
	$(MAKE) BUILDMODE=static -C $(CTRBDIR)luajit
	$(MAKE) -C $(CTRBDIR)lux
	$(MAKE) lib -C $(CTRBDIR)terra
	
#compiling the precompiled header
$(OBJSDIR)%.h.gch:%.h $(CMF)
	@dirname $@ | xargs mkdir -p 2>/dev/null || echo "$@ already exists" >/dev/null
	@echo "Precompiling header $@..."
	@$(CC) $(CFLAGS) ${CTRBSPTH:%=-I%} -o $@ -c $< || echo "error. Disabling precompiled header"
	@echo "...Done"

#compiling source files
$(OBJSDIR)%.o: %.c $(CMF)
	@dirname $@ | xargs mkdir -p 2>/dev/null || echo "$@ already exists" >/dev/null
	$(CC) $(CFLAGS) -I$(WORKDIR) $(CTRBSPTH:%=-I%) $(PCH_FILE:%.h=-include %.h) -c $< -o $@

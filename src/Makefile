
CC       := gcc
LD       := $(CC)
AR       := ar -cvq
RM       := rm -rf

CFLAGS   += -c -Wall -D_DEBUG

WORKDIR  := $(shell pwd)
OBJSDIR  := .objs/
CONTDIR  := ../contrib/

CONTRIBS := $(shell ls -d $(CONTDIR)*/) 

OUT      := zenith
SOURCES  := $(shell find -type f -name '*.c' -not -iwholename '*tests*' | cut -c 3-)
OBJECTS  := $(SOURCES:%.c=$(OBJSDIR)%.o)
PCH_FILE := stdafx.h
OBJSPCH  := $(PCH_FILE:%.h=$(OBJSDIR)%.h.gch)
CMF      := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

TSRC     := $(shell find -type f -name '*.c' -iwholename '*tests*' | cut -c 3-)
TOBJS    := $(TSRC:%.c=$(OBJSDIR)%.o)
TLIBS    := $(OUT) luajit dl terra pthread m
TOUT     := $(OUT)_test

#all
all: $(CONTRIBS) $(OBJSPCH) $(OBJECTS)
	$(AR) lib$(OUT).a $(OBJECTS)

#cleaning
.PHONY: clean
clean:
	$(RM) $(OBJSDIR) $(TOUT) lib$(OUT).a
	
.PHONY: clean-contrib
clean-contrib:
	$(foreach var,$(CONTRIBS),$(MAKE) clean -C $(var);)
	$(RM) $(CONTDIR)lib*.a
	
#tests
.PHONY: test
test: all $(TOBJS)
	$(LD) -o $(TOUT) $(TOBJS) -L$(WORKDIR) -L$(CONTDIR) $(TLIBS:%=-l%)

#compiling the contribs
.PHONY: contrib
contrib:
	$(MAKE) -C $@
	mv $@src/lib*.a $(CONTDIR)
	
$(CONTRIBS): $(CMF)
#TODO check if the lib already exists
	$(MAKE) -C $@
	mv $@src/lib*.a $(CONTDIR)
	
#compiling the precompiled header
$(OBJSDIR)%.h.gch:%.h $(CMF)
	@dirname $@ | xargs mkdir -p 2>/dev/null || echo "$@ already exists" >/dev/null
	@echo "Precompiling header $@..."
	@$(CC) $(CFLAGS) $(CONTRIBS:%=-I%src) -o $@ -c $< || echo "error. Disabling precompiled header"
	@echo "...Done"

#compiling source files
$(OBJSDIR)%.o: %.c $(CMF)
	@dirname $@ | xargs mkdir -p 2>/dev/null || echo "$@ already exists" >/dev/null
	$(CC) $(CFLAGS) -I$(WORKDIR) $(CONTRIBS:%=-I%src) $(PCH_FILE:%.h=-include %.h) -c $< -o $@

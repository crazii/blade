#!/usr/bin/env make -f

#GNU makefile for package generation

ifndef SourceDir
$(error $$(SourceDir) not defined.)
endif

ifndef OutDir
$(error $$(OutDir) not defined.)
endif

ifndef OutFile
$(error $$(OutFile) not defined.)
endif


ifeq ($(TERM),cygwin)
SourceDir := $(shell cygpath -u -p -a '$(SourceDir)')
OutDir := $(shell cygpath -u -p -a '$(OutDir)')
OutFile := $(shell cygpath -u -p -a '$(OutFile)')
endif


##############################################################################################
# env setup
##############################################################################################

#pwd Source/ as input dir

DEFAULT_LANGUAGE = en_US

LANGBUILD = BladeLangBuild

LANGBUILD_FLAGS = --lang=$(DEFAULT_LANGUAGE) --skip="backup" --output="$(OutDir)."


##############################################################################################
# source files
##############################################################################################

SOURCE_EXTENSION = cc cxx cpp h hpp rc rc2

FINDFLAGS = -name $(foreach i,$(SOURCE_EXTENSION), "*.$(i)" -or -name) ""
SOURCEFILES = $(shell find $(SourceDir)/ $(FINDFLAGS) )

#$(error $(SOURCEFILES) )

TARGET_FILE = $(OutDir)/$(DEFAULT_LANGUAGE)

##############################################################################################
# rules
##############################################################################################

all: $(TARGET_FILE)
	

$(TARGET_FILE) : $(SOURCEFILES)
	@echo $(LANGBUILD) $(LANGBUILD_FLAGS) .
	$(LANGBUILD) $(LANGBUILD_FLAGS) .
#!/usr/bin/env make -f

#GNU makefile for textures compression

ifndef OutDir
$(error $$(OutDir) not defined.)
endif

ifndef Format
$(error $$(Format) not defined.)
endif

ifndef Platform
$(error $$(Platform) not defined.)
endif

ifndef SUBDIRS
$(error $$(SUBDIRS) not defined.)
endif

ifndef ROOTDIR
ROOTDIR = $(PWD)/../art/textures
endif

ifeq ($(TERM),cygwin)
OutDir := $(shell cygpath -u -p -a '$(OutDir)')
ROOTDIR := $(shell cygpath -u -p -a '$(ROOTDIR)')
endif


ifndef MIPMAPS
MIPMAPS = -1
endif

ifndef SCALE_FACTOR
SCALE_FACTOR = 1.0
endif

##############################################################################################
# env setup
##############################################################################################
#$(info $$ROOTDIR = ${ROOTDIR})
#$(info $$SUBDIRS = ${SUBDIRS})
VPATH = $(shell find $(ROOTDIR) -type d)
COMMA :=,

SOURCETYPELIST := tga,bmp,png,dds
NORMALDIR := /normal/

TC = BladeTexCompressor
TCFLAGS = --target=$(Platform) --format=$(Format) --mipmaps=$(MIPMAPS) --scale=$(SCALE_FACTOR) #--verbose

##############################################################################################
# source files
##############################################################################################

SOURCE_EXTENSION = $(subst $(COMMA), ,$(SOURCETYPELIST))
SOURCE_EXTENSION := $(shell echo $(SOURCE_EXTENSION) | tr A-Z a-z )
SOURCE_EXTENSION := $(SOURCE_EXTENSION) $(shell echo $(SOURCE_EXTENSION) | tr a-z A-Z )
SUBDIRS := $(addprefix $(ROOTDIR)/, $(SUBDIRS))

FINDFLAGS = -name $(foreach i,$(SOURCE_EXTENSION), "*.$(i)" -or -name) ""
SOURCEFILES = $(shell find $(SUBDIRS) $(FINDFLAGS) )
SOURCEFILES += $(shell find $(ROOTDIR)/ -maxdepth 1 $(FINDFLAGS) )
SOURCEFILES := $(subst $(ROOTDIR)/,,$(SOURCEFILES))

TARGETFILES = $(addsuffix .$(Format),$(basename $(SOURCEFILES)))
TARGETFILES := $(addprefix $(OutDir),$(TARGETFILES))

##############################################################################################
# rules
##############################################################################################

all: $(TARGETFILES)
	

define compress_rule
$$(OutDir)%.$$(Format) : $(ROOTDIR)/%.$1
	@echo $$(TCFLAGS) $$< $$(subst $$(NORMALDIR),--normalmap,$$(findstring $$(NORMALDIR),$$<)) --output=$$(@D)
	@$$(TC) $$(TCFLAGS) $$< $$(subst $$(NORMALDIR),--normalmap,$$(findstring $$(NORMALDIR),$$<)) --output=$$(@D)
endef 

$(foreach EXT,$(SOURCE_EXTENSION),$(eval $(call compress_rule,$(EXT))))


#!/usr/bin/env make -f

#GNU makefile for package generation

FILTERS="*.recipe"

ifndef SourceDir
$(error $$(SourceDir) not defined.)
endif

ifndef OutFile
$(error $$(OutFile) not defined.)
endif

ifndef Platform
$(error $$(Platform) not defined.)
endif

ifeq ($(TERM),cygwin)
SourceDir := $(shell cygpath -u -p -a '$(SourceDir)')
OutFile := $(shell cygpath -u -p -a '$(OutFile)')
#$(info $$(SourceDir)=$(SourceDir))
endif

##############################################################################################
# env setup
##############################################################################################

#VPATH = $(shell find . -type d)

#pwd: Bin\Data
SOURCE_PLATFORM_ROOT = Data_Platform
SOURCE_FILES = $(shell find $(SourceDir) -type d -name "*" -maxdepth 1)
SOURCE_FILES := $(subst $(SOURCE_PLATFORM_ROOT),,$(SOURCE_FILES))
SOURCE_FILES := $(filter-out $(SourceDir),$(SOURCE_FILES))

#sub folders in SOURCE_PLATFORM_ROOT

DataFolders_Win32 = image/dx_gl shader/dx9
DataFolders_x64 = $(DataFolders_Win32)

DataFolders_Android = image/gles shader/gles3
DataFolders_iOS = $(DataFolders_Android)

#check if new platform added
ifndef DataFolders_$(Platform)
$(error $$(DataFolders_$(Platform)) not defined.)
endif

PACK = BPK
PACKFLAGS = 

##############################################################################################
# source files
##############################################################################################

#use by dependency only
PLATFORM_DIRS =$(DataFolders_$(Platform))
ROOT_FILES = $(shell find $(SourceDir) -maxdepth 1 -type f -name "*")
ROOT_FILES := $(filter-out $(SourceDir),$(ROOT_FILES))
SOURCE_FILES += $(ROOT_FILES)

FIND_PLATFORM_FOLDERS = $(addprefix  $(SourceDir)/$(SOURCE_PLATFORM_ROOT)/,$(PLATFORM_DIRS)) $(SOURCE_FILES)
#ALLFILES = $(shell find $(FIND_PLATFORM_FOLDERS) -type f -name "*")
#we shoulde include all sub folders, in case files are deleted
ALLFILES = $(shell find $(FIND_PLATFORM_FOLDERS) -name "*" -not -name '*.recipe' )
ALLFILES := $(filter-out $(SourceDir),$(ALLFILES))
ALLFILES += $(SOURCE_FILES)
SOURCE_FILES := $(subst $(SourceDir),,$(SOURCE_FILES))


##############################################################################################
# rules
##############################################################################################



all : $(OutDir)$(OutFile)
	

$(OutDir)$(OutFile) : $(ALLFILES)
	@echo $(PACKFLAGS) $(SOURCE_FILES) --rootdir=$(SourceDir) --output=$@
	$(PACK) $(PACKFLAGS) $(SOURCE_FILES) --rootdir=$(SourceDir) --output=$@
	@$(foreach SUBDIR,$(PLATFORM_DIRS), echo $(PACKFLAFGS) $(SOURCE_PLATFORM_ROOT)/$(SUBDIR) --append --rootdir=$(SourceDir) --destpath=/$(dir $(SUBDIR)) --filter="$(FILTERS)" --output=$@; )
	@$(foreach SUBDIR,$(PLATFORM_DIRS), $(PACK) $(PACKFLAFGS) $(SOURCE_PLATFORM_ROOT)/$(SUBDIR) --append --rootdir=$(SourceDir) --destpath=/$(dir $(SUBDIR)) --filter="$(FILTERS)" --output=$@; )
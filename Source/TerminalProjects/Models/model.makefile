#!/usr/bin/env make -f

#GNU makefile for build models: mesh & skeletal animations

ifndef OutDir
$(error $$(OutDir) not defined.)
endif

#actually we don't need platform here, since all model/skeleton resources should be cross platform
ifndef Platform
$(error $$(Platform) not defined.)
endif

ifndef ROOTDIR
ROOTDIR := $(PWD)/../art/models
endif

ifeq ($(TERM),cygwin)
OutDir := $(shell cygpath -u -p -a '$(OutDir)')
ROOTDIR := $(shell cygpath -u -p -a '$(ROOTDIR)')
endif

##############################################################################################
# env setup
##############################################################################################
#$(info $$OutDir = ${OutDir})
#$(info $$ROOTDIR = ${ROOTDIR})
VPATH = $(shell find $(ROOTDIR) -type d)
#$(info $$VPATH = ${VPATH})

COMMA :=,
SOURCETYPELIST := fbx

TARGET_FILE_TYPE = blm
TARGET_SKELETON_TYPE = bls


#mesh with skeletal animation
ANIMATED_DIR = animated

#static mesh or mesh with external skeleton
STATIC_MESH_DIR = static

#skeleton files only
SKELETON_DIR = skeleton

MC = BladeModelConverter
MCFLAGS = 

##############################################################################################
# source files
##############################################################################################

SOURCE_EXTENSION = $(subst $(COMMA), ,$(SOURCETYPELIST))
SOURCE_EXTENSION := $(shell echo $(SOURCE_EXTENSION) | tr A-Z a-z )
SOURCE_EXTENSION := $(SOURCE_EXTENSION) $(shell echo $(SOURCE_EXTENSION) | tr a-z A-Z )
SUBDIRS = $(shell find $(ROOTDIR) -type d -maxdepth 1)

#default files: mesh & skeletal animation
ANIMATED_FINDFLAGS = -maxdepth 2 -type f -wholename $(foreach i,$(SOURCE_EXTENSION), "*$(ANIMATED_DIR)/*.$(i)" -or -wholename) ""
ANIMATED_FILES = $(shell find $(SUBDIRS) $(ANIMATED_FINDFLAGS) )
ANIMATED_FOLDER_FINDFLAGS = -maxdepth 2 -type d -wholename $(foreach i,$(SOURCE_EXTENSION), "*$(ANIMATED_DIR)/*" -or -wholename) ""
ANIMATED_FILES = $(shell find $(SUBDIRS) $(ANIMATED_FOLDER_FINDFLAGS) )

ANIMATED_TARGETS = $(addsuffix .$(TARGET_FILE_TYPE),$(basename $(ANIMATED_FILES)))
ANIMATED_TARGETS += $(addsuffix .$(TARGET_SKELETON_TYPE),$(basename $(ANIMATED_FILES)))
ANIMATED_TARGETS := $(addprefix $(OutDir), $(ANIMATED_TARGETS:$(ROOTDIR)/%=%))

#skeleton files
SKELETON_FINDFLAGS = -maxdepth 2 -type f -wholename $(foreach i,$(SOURCE_EXTENSION), "*$(SKELETON_DIR)/*.$(i)" -or -wholename) ""
SKELETON_FILES = $(shell find $(SUBDIRS) $(SKELETON_FINDFLAGS) )
SKELETON_FOLDER_FINDFLAGS = -maxdepth 2 -type d -wholename $(foreach i,$(SOURCE_EXTENSION), "*$(SKELETON_DIR)/*" -or -wholename) ""
SKELETON_FILES += $(shell find $(SUBDIRS) $(SKELETON_FOLDER_FINDFLAGS) )
SKELETON_TARGETS = $(addsuffix .$(TARGET_SKELETON_TYPE),$(basename $(SKELETON_FILES)))
SKELETON_TARGETS := $(addprefix $(OutDir),$(SKELETON_TARGETS:$(ROOTDIR)/%=%))

#static mesh, or mesh with external skeleton files
STATIC_FINDFLAGS = -maxdepth 2 -type f -wholename $(foreach i,$(SOURCE_EXTENSION), "*$(STATIC_MESH_DIR)/*.$(i)" -or -wholename) ""
STATIC_FILES = $(shell find $(SUBDIRS) $(STATIC_FINDFLAGS) )
STATIC_FOLDER_FINDFLAGS = -maxdepth 2 -type d -wholename $(foreach i,$(SOURCE_EXTENSION), "*$(STATIC_MESH_DIR)/*" -or -wholename) ""
STATIC_FILES += $(shell find $(SUBDIRS) $(STATIC_FOLDER_FINDFLAGS) )
STATIC_TARGETS = $(addsuffix .$(TARGET_FILE_TYPE),$(basename $(STATIC_FILES)))
STATIC_TARGETS := $(addprefix $(OutDir),$(STATIC_TARGETS:$(ROOTDIR)/%=%))


#TARGETFILES = $(ANIMATED_TARGETS) $(SKELETON_TARGETS) $(STATIC_TARGETS)
#$(error $(TARGETFILES))
#$(error $(addprefix %/*., $(SOURCE_EXTENSION)))

##############################################################################################
# rules
##############################################################################################

all: $(ANIMATED_TARGETS) $(SKELETON_TARGETS) $(STATIC_TARGETS)


add_flags = $(subst /$(STATIC_MESH_DIR)/,--skeleton=false,$(findstring /$(STATIC_MESH_DIR)/,$(1))) $(subst /$(SKELETON_DIR)/,--mesh=false,$(findstring /$(SKELETON_DIR)/,$(1)))

define all_types_rule

#grouped source in folder to merge into one file
#note: $1 cannot share between expansions, so *.FBX & *.fbx will generate two diffferent rules.
#but here we need one unique rules for all files with mixed extensions. HOWTO:
$$(OutDir)%.$$(TARGET_FILE_TYPE) $$(OutDir)%.$$(TARGET_SKELETON_TYPE) : $(ROOTDIR)/%/*.$1 
		@echo $$(MC) $$^ $$(MCFLAGS) $$(call add_flags,$$<) --output=$(OutDir)$$*.$(TARGET_FILE_TYPE)
		$$(MC) $$^ $$(MCFLAGS) $$(call add_flags,$$<) --output=$(OutDir)$$*.$(TARGET_FILE_TYPE)
		

#separated files
$$(OutDir)%.$$(TARGET_FILE_TYPE) $$(OutDir)%.$$(TARGET_SKELETON_TYPE) : %.$1
		@echo $$(MC) $$< $$(MCFLAGS) $$(call add_flags,$$<) --output=$(OutDir)$$*.$(TARGET_FILE_TYPE)
		$$(MC) $$< $$(MCFLAGS) $$(call add_flags,$$<) --output=$$(OutDir)$$*.$(TARGET_FILE_TYPE)
endef 

$(foreach EXT,$(SOURCE_EXTENSION),$(eval $(call all_types_rule,$(EXT))))

	
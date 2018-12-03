#!/usr/bin/env make -f

#GNU makefile for copying files needed by build APK

#copy files from project folder into TargetDir to prepare APK build.
#the application uses some files from Foudation/Framework, i.e. some Java files.

ifndef TargetDir
$(error $$(TargetDir) not defined.)
endif

ifndef ExtraDir
$(error $$(ExtraDir) not defined.)
endif

ifndef TargetLib
$(error $$(TargetLib) not defined.)
endif
TargetLib := $(TargetLib:lib%.so=%)

ifeq ($(TERM),cygwin)
TargetDir := $(shell cygpath -u -p -a '$(TargetDir)')
ExtraDir := $(shell cygpath -u -p -a '$(ExtraDir)')
endif

#$(info $$TargetDir = ${TargetDir})
#$(info $$ExtraDir = ${ExtraDir})

all: all_files


##############################################################################################
# env setup
##############################################################################################


##############################################################################################
# source files
##############################################################################################
SOURCES := $(shell find ./ -type f)
SOURCES := $(subst ./,,$(SOURCES))
SOURCES := $(filter-out CopyFiles.makefile, $(SOURCES))
#$(info $$SOURCES = ${SOURCES})
TARGET_FILES = $(addprefix $(TargetDir)/,$(SOURCES))

EXTRA_SOURCES := $(shell find $(ExtraDir) -type f)
EXTRA_FILES = $(subst $(ExtraDir),,$(EXTRA_SOURCES))

TARGET_SOURCE_DIR = $(dir $(word 1, $(filter %.java,$(TARGET_FILES))))
#$(info $$(TARGET_SOURCE_DIR)=$(TARGET_SOURCE_DIR))
EXTRA_FILES  := $(addprefix $(TARGET_SOURCE_DIR),$(EXTRA_FILES))
#$(info $$(EXTRA_FILES)=$(EXTRA_FILES))

#rules

$(TargetDir)/%/AndroidManifest.xml: ./%/AndroidManifest.xml
	@echo "$< -> $@"
	@mkdir --parents $(dir $@)
	@sed -e 's/$$(TARGET_SO_LIB)/$(TargetLib)/' $< > $@

$(TargetDir)/% : ./%
	@echo "$< -> $@"
	@mkdir --parents $(dir $@)
	@cp $< $@

$(EXTRA_FILES): $(EXTRA_SOURCES)
	@echo "$(ExtraDir)/$(notdir $@) -> $@"
	@mkdir --parent $(dir $@)
	@sed -e 's/$$(TARGET_SO_LIB)/$(TargetLib)/' $(ExtraDir)/$(notdir $@) > $@
	#@cp $(ExtraDir)/$(notdir $@) $(TARGET_SOURCE_DIR)


##############################################################################################
# rules
##############################################################################################

all_files: $(TargetDir) $(TARGET_FILES) $(EXTRA_FILES)
	

$(TargetDir):
	@mkdir $(TargetDir)


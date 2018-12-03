#!/usr/bin/env make -f

#GNU makefile for textures compression

ifndef LANG
LANG = enUS
endif

SUBDIRS =  $(addprefix lang/common/, $(LANG))

include Shared.inc

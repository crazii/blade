#!/usr/bin/env make -f

#GNU makefile for textures compression

ifndef LANG
LANG = enUS
endif

SUBDIRS =  $(addprefix lang/UI/, $(LANG))
MIPMAPS := 1

include Shared.inc

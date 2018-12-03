#!/usr/bin/env make -f

#GNU makefile for textures compression

SUBDIRS := terrain_textures

ifeq "$(Platform)" "Android"
SCALE_FACTOR := 0.5
endif

include Shared.inc

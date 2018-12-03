#!/usr/bin/env make -f

#GNU makefile for textures compression

FILES = $(shell find -maxdepth 1 -name "*.makefile" )
FILES := $(filter-out MakeAll.makefile ./MakeAll.makefile,$(FILES))

.PHONY: makeall

makeall:
	@$(foreach FILE,$(FILES), $(MAKE) -f $(FILE); )
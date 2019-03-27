BUILDROOT ?= build
BUILDTYPE ?= release
include yama.mk

-load $(wildcard dbdeps$(yama.Dll))
$(call yama.subdirs,updbdeps plugin)

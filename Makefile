BUILDROOT ?= build
BUILDTYPE ?= release
include yama.mk

-load dbdeps$(yama.Dll)
dbdeps$(yama.Dll) :

$(call yama.subdirs,updbdeps plugin)

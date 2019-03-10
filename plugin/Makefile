BUILDROOT ?= build
BUILDTYPE ?= release
include yama.mk


LMDBPATH := ../external/lmdb/libraries/liblmdb


# set compilation flags
CPPFLAGS += -MMD -MP
CPPFLAGS += -I$(call yama.pathConcatOrDot,$(yama.Source),$(LMDBPATH))
LDFLAGS += -Wl,--gc-sections,--exclude-libs,lmdb.a


# define our target
all : plugin
$(call yama.goalDll,plugin,plugin wrapdb,dbdeps)
$(call yama.rules)


# link with lmdb
$(call yama.subdir,$(if $(yama.Level),../lmdb,lmdb),$(LMDBPATH),lmdb.yama)
$(plugin) : $(lmdb)


# auto dependencies
$(call yama.deps,$(plugin.Objects) $(lmdb.Objects))
# manual dependencies
#$(plugin.Objects) : $(yama.Source)wrapdb.h
#$(yama.Build)wrapdb$(yama.O) : $(yama.Source)$(LMDBPATH)/lmdb.h

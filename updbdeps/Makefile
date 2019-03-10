BUILDROOT ?= build
BUILDTYPE ?= release
include yama.mk


# set compilation flags
REFLAGS += -Wno-nondeterministic-tags
CPPFLAGS += -MMD -MP
CPPFLAGS += -I$(call yama.pathConcatOrDot,$(yama.Source),../plugin)


# define our target
all : updbdeps
$(call yama.goalExe,updbdeps,main getdelim)
$(call yama.rules,,,c re)
.SECONDARY : $(yama.Build)main.c


# link with plugin
$(call yama.subdir,$(if $(yama.Level),../plugin,plugin),../plugin)
$(updbdeps) : $(plugin)


# auto dependencies
$(call yama.deps,$(updbdeps.Objects))
# manual dependencies
#$(yama.Build)main$(yama.O) : $(yama.Source)../plugin/wrapdb.h

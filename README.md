dbdeps
======

GNU Make dependency database plugin
-----------------------------------

This is a _GNU Make_ dependency database plugin inspired by a similar functionality in _Ninja_ (cf. `.ninja_deps`). It can reduce the startup time of the projects consisting of an extremely large number of source files by merging all `.d` files into a single database.

An example usage:

```
ifneq (,$(filter load,$(.FEATURES)))  # load supported
    DEPDIR := .deps                   # output directory
    CPPFLAGS += -MMD -MP              # compiler flags to generate dependencies
    load dbdeps.dll                   # load plugin
    $(dbdeps $(DEPDIR))               # read in all dependencies
    all : $(DEPDIR)/data.mdb          # mark database as a target for update
    # the rules to update database from .d files
    # note: double colon is needed for splitting a single rule into several ones
    $(DEPDIR)/data.mdb :: $(objects1) ;@updbdeps --remove $(?:.o=.d) -d $(@D)
    $(DEPDIR)/data.mdb :: $(objects2) ;@updbdeps --remove $(?:.o=.d) -d $(@D)
endif
```

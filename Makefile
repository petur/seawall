-include settings.mk

ifeq ($(ARCH),)
ARCH := native
endif
RTTIFLAGS = -fno-rtti -fno-exceptions
CXXFLAGS += -Wall -Wextra -Werror -std=c++17 -O3 -ffast-math -march=$(ARCH) -mtune=$(ARCH) -flto $(RTTIFLAGS) $(PGOFLAGS)

version :=
ifeq ($(version),)
version := $(shell date '+%Y%m%d')
commit := $(shell git rev-parse --short HEAD)
ifneq ($(commit),)
version := $(version)-$(commit)
branch := $(shell git branch --show-current)
ifneq ($(branch),)
ifneq ($(branch),main)
version := $(version)-$(branch)
endif
endif
endif
endif

release := out/seawall-$(version)$(SUFFIX)
CPPFLAGS += -DSEAWALL_VERSION=$(version)

ifeq ($(LLVM_PROFDATA),)
LLVM_PROFDATA=llvm-profdata
endif

all:	test $(release)

ifneq ($(branch),)
branchlink := branches/seawall-$(branch)

all:	$(branchlink)

$(branchlink):	$(release) branches
	ln -s -f ../$(release) $(branchlink)
endif

$(release):	seawall out
	cp $< $@

seawall:	seawall.cc | profile
	$(RM) profile/*
	$(MAKE) PGOFLAGS=-fprofile-generate=./profile profile/seawall
	./profile.sh profile/seawall
	$(RM) profile/seawall
	if $(CXX) --version | grep -q clang; then $(LLVM_PROFDATA) merge -o ./profile/default.profdata ./profile; fi
	$(MAKE) PGOFLAGS=-fprofile-use=./profile profile/seawall
	mv profile/seawall $@

profile/seawall:	seawall.cc
	$(LINK.cc) $^ -o $@

branches out profile:
	mkdir -p $@

test:	seawall
	./test.sh

tune:	seawall.tune
	./seawall.tune < filtered/20241124-171359.csv

seawall.tune:	CPPFLAGS += -DTUNE=1
seawall.tune:	RTTIFLAGS =
seawall.tune:	LDLIBS += -ltbb
seawall.tune:	seawall.cc
	$(LINK.cc) $^ $(LDLIBS) -o $@

clean:
	$(RM) -r seawall seawall.tune profile

.PHONY:	all test tune clean

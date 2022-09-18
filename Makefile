ifeq ($(ARCH),)
ARCH := native
endif
CXXFLAGS += -Wall -Wextra -Werror -std=c++17 -Ofast -march=$(ARCH) -mtune=$(ARCH) -flto -fno-rtti -fno-exceptions -fgcse-sm -fgcse-las $(PGOFLAGS)
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
release := out/seawall-$(version)$(SUFFIX)
CPPFLAGS += -DSEAWALL_VERSION=$(version)

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
	$(MAKE) PGOFLAGS=-fprofile-use=./profile profile/seawall
	mv profile/seawall $@

profile/seawall:	seawall.cc
	$(LINK.cc) $^ -o $@

branches out profile:
	mkdir -p $@

test:	seawall
	./test.sh

tune:	seawall.tune
	cat samples/*.csv | ./seawall.tune

seawall.tune:	CPPFLAGS += -DTUNE=1
seawall.tune:	seawall.cc
	$(LINK.cc) $^ -o $@

clean:
	$(RM) -r seawall seawall.tune profile

.PHONY:	all test tune clean

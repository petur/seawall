ifeq ($(ARCH),)
ARCH := native
endif
CXXFLAGS += -Wall -Wextra -Werror -std=c++17 -Ofast -march=$(ARCH) -mtune=$(ARCH) -flto -fno-rtti -fno-exceptions -fgcse-sm -fgcse-las
version := $(shell date '+%Y%m%d')-$(shell git rev-parse --short HEAD)
branch := $(shell git branch --show-current)
ifneq ($(branch),main)
version := $(version)-$(branch)
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
	cp seawall $(release)

seawall:	seawall.cc

branches out:
	mkdir -p $@

test:	seawall
	./test.sh

tune:	seawall.tune
	cat samples/*.csv | ./seawall.tune

seawall.tune:	CPPFLAGS += -DTUNE=1
seawall.tune:	seawall.cc
	$(LINK.cc) $^ -o $@

clean:
	$(RM) -r seawall seawall.tune

.PHONY:	all test tune clean

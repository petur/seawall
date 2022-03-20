CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -march=native -mtune=native
branch := $(shell git branch --show-current)
release := out/seawall.$(branch)

all:	test $(release)

$(release):	seawall out
	cp seawall $(release)

seawall:	seawall.cc

out:
	mkdir -p out

test:	seawall
	./test.sh

clean:
	$(RM) -r seawall out

.PHONY:	clean

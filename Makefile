CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -O3 -march=native -mtune=native
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

tune:	seawall.tune
	cat samples/*.csv | ./seawall.tune

seawall.tune:	seawall.cc
	$(CXX) $(CXXFLAGS) -DTUNE=1 -o $@ $^

clean:
	$(RM) -r seawall seawall.tune out

.PHONY:	clean

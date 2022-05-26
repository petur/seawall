CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -march=native -mtune=native

all:	test

seawall:	seawall.cc

test:	seawall
	./test.sh

clean:
	$(RM) seawall

.PHONY:	clean

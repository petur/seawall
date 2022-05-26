CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -march=native -mtune=native

all:	seawall

seawall:	seawall.cc

clean:
	$(RM) seawall

.PHONY:	clean

CPP   = g++-4.7
FLAGS	= -std=c++11 -g -Wall -Werror
LIBS	= -pthread



all: everyTM

everyTM: everyTM.cc
	$(CPP) $(FLAGS) $(LIBS) everyTM.cc -o everyTM

clean:
	rm -f everyTM


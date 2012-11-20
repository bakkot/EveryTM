CPP   = g++-4.7
FLAGS	= -std=c++11 -g -Wall -Werror
LIBS	= 



all: machine thread

machine: machine.cc
	$(CPP) $(FLAGS) $(LIBS) machine.cc -o machine

thread: thread.cc
	$(CPP) $(FLAGS) $(LIBS) thread.cc -o thread

clean:
	rm -f machine thread


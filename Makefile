CPP   = g++-4.7
FLAGS	= -std=c++11 -g -Wall -Werror
LIBS	= 



all: everyTM

everTM: everTM.cc
	$(CPP) $(FLAGS) $(LIBS) everTM.cc -o everTM

clean:
	rm -f everTM


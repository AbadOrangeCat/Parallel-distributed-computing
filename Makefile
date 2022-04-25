aa.out : a.o c.o
	g++ -o aa.out a.cpp
a.o : a.cpp a.h
	g++ -o a.o a.cpp
c.o : c.cpp c.h
	g++ -o c.o c.cpp
all: CPU

CPU: main_CPU.o CPU.o stack.o
	g++ main_CPU.o CPU.o stack.o -o proc -g

main_CPU.o: main_CPU.cpp
	g++ -c main_CPU.cpp

stack.o: stack.cpp
	g++ -c stack.cpp

CPU.o: CPU.cpp
	g++ -c CPU.cpp

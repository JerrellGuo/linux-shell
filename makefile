EXECUTABLE=mysh
CPPFLAGS= -std=c99 -g -O3 -Wall

all: $(EXECUTABLE)

$(EXECUTABLE): mysh.o execute.o globals.o parser.o 
	gcc $(CPPFLAGS) mysh.o execute.o globals.o parser.o  -o $(EXECUTABLE)

execute.o: execute.c mysh.h
	gcc $(CPPFLAGS) -c execute.c 

globals.o: globals.c mysh.h 
	gcc $(CPPFLAGS) -c globals.c 

parser.o: parser.c mysh.h 
	gcc $(CPPFLAGS) -c parser.c

mysh.o: mysh.c mysh.h 
	gcc $(CPPFLAGS) -c mysh.c

clean: 
	rm -f *.o $(EXECUTABLE)

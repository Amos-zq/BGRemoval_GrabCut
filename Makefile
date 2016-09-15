# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
CC = g++
CFLAGS  = `pkg-config --cflags --libs opencv`


default: main autoclean


main:  main.o gcapp.o basic_define.o frame_by_frame.o floodfill.o 
	$(CC) -o main main.o gcapp.o basic_define.o frame_by_frame.o floodfill.o $(CFLAGS) 


main.o:  main.cpp gcapp.h 
	$(CC) $(CFLAGS) -c main.cpp


gcapp.o:  gcapp.cpp gcapp.h 
	$(CC) $(CFLAGS) -c gcapp.cpp


basic_define.o:  basic_define.cpp gcapp.h 
	$(CC) $(CFLAGS) -c basic_define.cpp
	
	
frame_by_frame.o:  frame_by_frame.cpp gcapp.h 
	$(CC) $(CFLAGS) -c frame_by_frame.cpp


floodfill.o:  floodfill.cpp gcapp.h 
	$(CC) $(CFLAGS) -c floodfill.cpp


autoclean:
	rm -f *.o *~

clean: 
	rm -f main *.o *~

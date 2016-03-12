# Linux
CC=g++
LIBS= -lglut -lGLEW -lGL -lGLU

# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler

# Compiler flags
CXXFLAGS= -g -Wall -std=c++0x

all: ../bin/Simulation 

../bin/Simulation: ../src/main.cpp
	$(CC) $(CXXFLAGS) ../src/main.cpp ../src/Universe.h ../src/Universe.cpp ../src/Firebox.h ../src/Firebox.cpp ../src/Lunchbox.h ../src/Lunchbox.cpp ../src/Squib.h ../src/Squib.cpp -o ../bin/Simulation $(LIBS)

clean:
	rm -rf *.o ../bin/Simulation

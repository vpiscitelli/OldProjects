PA11 - Labyrinth
============================
-Group
	Zeeshan Sajid, Vance Piscitelli

Note: This is the program that should be graded.

Extra Credit
------------
For extra credit, we have sound effects, a top 10 scores history, multiple balls, multiple levels, day/night mode for difficulty, walls that can change height based on distance from ball, and the ability to load custom levels from text files.

Compile
-------
- cd build; make clean, make

Run 
---
- cd bin; ./bin/Lab
- run from /bin

Easy Compiling/Running notes
----------------------------
To compile from the main project folder:
	make -C ./build
To compile from the bin folder:
	make -C ../build
To run the executable from the bin folder:
	./Lab

Controls
--------
	Main Menu: Press Q to start the game
	Top Scores: Press 9 to go and see top scorese
	
	Right-click:
		Brings up our menu: 
				-Start- Unpauses the game, so spheres and collisions will start working again
				-Stop- Pauses the game while playing, ball does not move
				-Quit- This quits the game, essentially top scores will be written to a file prior to exiting

	Keyboard:
			'Arrow: Up key: Tilts the board upward, and placing gravity upon the sphere towards the -z axis
			Keys'	Down key: Tils board downward, and placing gravity upon the sphere towards the z axis
					Right key: Tilts the board to the right, and placing gravity upon the sphere towards the x axis
					Left key: Tilts the board to the left, and placing gravity upon the sphere towards the -x axis
			'9': Goes to the top Score menu, only if on title Screen
			'r': Makes the sphere reset to the original starting point within the specific level
			'q': Starts the game, if and only if on the title screen
			'n': Goes to the next level while at the same time dynamically changing each and every individual cube.
			'2': Difficulty level of sorts, in which the player cannot see the maze and will have to use their wits for the dynamically changing cubes
			'4': Changes the theme of the game into a red and gold setting in which there is only one source of light and that is the spotlight
			'5': Changes the theme back to the original in which there are multiple light sources, the path is visiible 
			'3': Creates the second ball for the player to manipulate and guide through the maze. Is toggleable in which it will disppear and reappear when clicked 
			'1': Toggles day/night mode

		Esc Key: Quits the game entirely

Magick++ Installation Instructions
----------------------------------

In your .cpp file, include:
#include <Magick++.h>

And initialize in main:
Magick :: InitializeMagick(*argv);

In the makefile, add LDFLAGS = `Magick++-config --cppflags --cxxflags --ldflags --libs`
and then add LDFlAGS to the compilation line like this :
$(CC) $(CXXFLAGS) ../src/shader.cpp ../src/main.cpp -o ../bin/Lab $(LDFLAGS)

Also, don't forget to add -lMagick++ to the LIBS to use. 

Install these libraries with sudo apt-get install:
(These are the libraries installed on the ECC computers)
libgraphicsmagick3
libgraphicsmagick1-dev
libgraphics-magick-perl
libgraphicsmagick++1-dev
libmagickcore5-extra
libmagickwand5
imagemagick
libgraphicsmagick++3
graphicsmagick-libmagick-dev-compat
libmagickcore5
imagemagick-common

Or you can follow the installation instructions from:
http://www.imagemagick.org

See Technical Manual and User Manual for more information!

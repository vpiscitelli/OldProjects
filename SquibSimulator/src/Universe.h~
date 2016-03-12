// Universe.h

#ifndef UNIVERSE_H
#define UNIVERSE_H

#include "Firebox.h"
#include <iostream>

using namespace std;

class Universe
{
	public:
		Universe();
		Universe(bool);
		Universe(int, int, int, bool);
		
		~Universe();

		void setActive(bool);
		bool getActive();

		void setFireboxAddress(int, int);
		int getFireboxAddress(int);

		void setLunchboxAddress(int, int, int);
		int getLunchboxAddress(int, int);

		void setSquibAddress(int, int, int, int);
		int getSquibAddress(int, int, int);

		void setArmedSquib(int, int, int, bool);
		bool getArmedSquib(int, int, int);

		void setActiveSquib(int, int, int, bool);
		bool getActiveSquib(int, int, int);

		void fireSquib(int, int, int);
		void stopFire(int, int, int);
		bool getFiring(int, int, int);

		int getNumFireboxes();
		int getNumLunchboxes();
		int getNumSquibs();

		void display();

	private:
		bool active;
		int numFireboxes, numLunchboxes, numSquibs;
		Firebox fireboxList[16];
};

#endif

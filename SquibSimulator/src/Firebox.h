// Firebox.h

#ifndef FIREBOX_H
#define FIREBOX_H

#include "Lunchbox.h"
#include <iostream>

using namespace std;

class Firebox
{
	public:
		Firebox();
		Firebox(bool);
		Firebox(bool[12], bool);

		~Firebox();

		bool setPosition(int);
		int getPosition();

		void setActive(bool);
		bool getActive();

		void setAddress(int);
		int getAddress();

		void setLunchboxAddress(int, int);
		int getLunchboxAddress(int);

		void setSquibAddress(int, int, int);
		int getSquibAddress(int, int);

		void setArmedSquib(int, int, bool);
		bool getArmedSquib(int, int);

		void setActiveSquib(int, int, bool);
		bool getActiveSquib(int, int);

		void fireSquib(int, int);
		void stopFire(int, int);
		bool getFiring(int, int);

		void setActiveLunchboxes(int, int);

		void display();

	private:
		bool active;
		int position;
		int address;
		Lunchbox lunchboxList[12];
};

#endif

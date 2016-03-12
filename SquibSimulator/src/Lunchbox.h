// Lunchbox.h

#ifndef LUNCHBOX_H
#define LUNCHBOX_H

#include "Squib.h"
#include <iostream>

using namespace std;

class Lunchbox
{
	public:
		Lunchbox();
		Lunchbox(bool);
		Lunchbox(bool[8], bool); 

		~Lunchbox();

		bool setPosition(int);
		int getPosition();

		void setActive(bool);
		bool getActive();

		void setAddress(int);
		int getAddress();

		void setSquibAddress(int, int);
		int getSquibAddress(int);

		void setArmedSquib(int, bool);
		bool getArmedSquib(int);

		void setActiveSquib(int, bool);
		bool getActiveSquib(int);

		void setActiveSquibs(int);
		
		void fireSquib(int);
		void stopFire(int);
		bool getFiring(int);

		void display();

	private:
		bool active;
		int position;
		int address;
		Squib squibList[8];
};

#endif

// Squib.h

#ifndef SQUIB_H
#define SQUIB_H

#include <iostream>

using namespace std;

class Squib
{
	public:
		Squib();
		Squib(int, bool);
		Squib(int, int, bool);

		~Squib();

		void fire();
		void stopFire();
		bool getFiring();	
		int getNumFires();

		int getPosition();
		bool setPosition(int);

		void setActive(bool);
		bool getActive();

		void setArmed(bool);
		bool getArmed();

		void setAddress(int);
		int getAddress();

		void display();
	
	private:
		int position; // 0-7
		int address;
		int numFires; // 0 or more
		bool active;
		bool armed;
		bool firing;
};

#endif

#include "Universe.h"

Universe::Universe()
{
	active = true;

	numFireboxes = 16;
	numLunchboxes = 12;
	numSquibs = 8;

	for(int i = 0; i < 16; i++)
	{
		fireboxList[i].setPosition(i);
		fireboxList[i].setActive(true);
	}
}

Universe::Universe(bool enable)
{
	active = enable;

	numFireboxes = 16;
	numLunchboxes = 12;
	numSquibs = 8;

	for(int i = 0; i < 16; i++)
	{
		fireboxList[i].setPosition(i);
		fireboxList[i].setActive(enable);
	}
}

Universe::Universe(int numFb, int numLb, int numSb, bool enable)
{
	active = enable;

	numFireboxes = numFb;
	numLunchboxes = numLb;
	numSquibs = numSb;

	for(int i = 0; i < numFireboxes; i++)
	{
		fireboxList[i].setPosition(i);
		fireboxList[i].setActive(true);
		fireboxList[i].setActiveLunchboxes(numLunchboxes, numSquibs);
	}
}

Universe::~Universe()
{
	for(int i = 0; i < 16; i++)
	{
		fireboxList[i].setPosition(-1);
		fireboxList[i].setActive(false);
	}
}

void Universe::setActive(bool activate)
{
	active = activate;
}

bool Universe::getActive()
{
	return active;
}

void Universe::setFireboxAddress(int fPos, int add)
{
	fireboxList[fPos].setAddress(add);
}

int Universe::getFireboxAddress(int fPos)
{
	return fireboxList[fPos].getAddress();
}

void Universe::setLunchboxAddress(int fPos, int lPos, int add)
{
	fireboxList[fPos].setLunchboxAddress(lPos, add);
}

int Universe::getLunchboxAddress(int fPos, int lPos)
{
	return fireboxList[fPos].getLunchboxAddress(lPos);
}

void Universe::setSquibAddress(int fPos, int lPos, int sPos, int add)
{
	fireboxList[fPos].setSquibAddress(lPos, sPos, add);
}

int Universe::getSquibAddress(int fPos, int lPos, int sPos)
{
	return fireboxList[fPos].getSquibAddress(lPos, sPos);
}

void Universe::setArmedSquib(int fAdd, int lAdd, int sAdd, bool arm)
{
	for(int i = 0; i < 16; i++)
	{
		if(fireboxList[i].getAddress() == fAdd)
		{
			fireboxList[i].setArmedSquib(lAdd, sAdd, arm);
		}
	}
}

bool Universe::getArmedSquib(int fAdd, int lAdd, int sAdd)
{
	for(int i = 0; i < 16; i++)
	{
		if(fireboxList[i].getAddress() == fAdd)
		{
			return fireboxList[i].getArmedSquib(lAdd, sAdd);
		}
	}

	return false;
}

void Universe::setActiveSquib(int fAdd, int lAdd, int sAdd, bool arm)
{
	for(int i = 0; i < 16; i++)
	{
		if(fireboxList[i].getAddress() == fAdd)
		{
			fireboxList[i].setActiveSquib(lAdd, sAdd, arm);
		}
	}
}

bool Universe::getActiveSquib(int fAdd, int lAdd, int sAdd)
{
	for(int i = 0; i < 16; i++)
	{
		if(fireboxList[i].getAddress() == fAdd)
		{
			return fireboxList[i].getActiveSquib(lAdd, sAdd);
		}
	}

	return false;
}

void Universe::fireSquib(int fPos, int lPos, int sPos)
{
	fireboxList[fPos].fireSquib(lPos, sPos);
}

void Universe::stopFire(int fPos, int lPos, int sPos)
{
	fireboxList[fPos].stopFire(lPos, sPos);
}

bool Universe::getFiring(int fPos, int lPos, int sPos)
{
	return fireboxList[fPos].getFiring(lPos, sPos);
}

int Universe::getNumFireboxes()
{
	return numFireboxes;
}

int Universe::getNumLunchboxes()
{
	return numLunchboxes;
}

int Universe::getNumSquibs()
{
	return numSquibs;
}

void Universe::display()
{
	for(int i = 0; i < 12; i++)
	{
		fireboxList[i].display();
		cout << endl;
	}

}

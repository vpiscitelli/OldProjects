#include <iostream>
#include <chrono> // to keep track of time
#include "Universe.h" // for my representation of my system

// for visuals/windows
#include <GL/glew.h> 
#include <GL/glut.h>

#include <queue>
#include <vector>
#include <stdlib.h>

using namespace std;

Universe universe = NULL;


int timeStep = 0; // the current time step of the simulation
float simSpeed = 0.25; // how fast each loop in the simulation should be

bool pause = false;

int w = 1080, h = 720;// Window size

bool* keyStates = new bool[256]; // whether or not each key is pressed
bool* keySpecialStates = new bool[256]; // for the special keys

// valid mode options:
// n - Do nothing. just does other stuff
// t - Prints time step and just does other stuff
// r - arms random squib
// a - arms specified squib
// f - fires armed squibs
char mode = 'n';

// simulates the packet that is sent between the computer and the fireboxes
struct packet
{
	char type = -1;
	int fbAdd = -1;
	int lbAdd = -1;
	int sbAdd = -1;
	int numFb = 0;
	int numLb = 0;
	int numSb = 0;
	int bright = 0;
	int error; 
};

// a struct to hold the information of a squib being fired so
// program knows it will need to be turned off
struct fSquib
{
	int fPos;
	int lPos;
	int sPos;
	int fireCount = 4;
};

vector<fSquib > firingSquibs;

/**
	--TYPES OF PACKETS--

	arm packet-
		packet type, firebox address, lunchbox address, squib address, brightness

	fire packet-
		packet type, number of fireboxes, number of lunchboxes, number of squibs

	universe setup packet-
		packet type, number of fireboxes, number of lunchboxes, number of squibs
	
	readyPacket-
		packet type, firebox address, lunchbox address, squib address

	query packet-
		packet type, flags, firebox address

	response packet-
		packet type, flags, firebox address, lunchbox address

	squib error packet-
		packet type, flags, firebox address, lunchbox address, squib address
	
 	--HOW RS485 IS USED--

	packets are sent from the computer to the fireboxes
	using the Tx (2 lines)

	packets are sent back from devices to the computer
	using the Rx (2 lines)

	This will be simulated with a queue from the computer sending packets to the fireboxes
	and a queue from the fireboxes sending packets to the computer simultaneously
**/

queue <packet> cQueue; 	// keeps track of packets that need to be sent out from the computer.
						// should normally only have one packet in it at a time and it should
						// get sent out but exceptions occur.
queue <packet> fQueue; 	// keeps track of packets that need to be sent out from the fireboxes.
						// should normally only have one packet in it at a time and it should
						// get sent out but exceptions occur.

// just some brief info about the project
void intro();

// initializes the simulation
void init(int argc, char **argv);

// asks user to arm a specific squib
void armIndividualSquib();

// randomly selects a squib to arm
void armRandomSquib();

// sends out packet to arm a specific squib
void sendArm(int, int, int);

// sends out a packet to fire all armed squibs
void fireArmedSquibs();

// sends out packets to assign addresses to the fireboxes & squibs 
void assignAddresses();

// the firebox-side packet handler
void fph(packet);

// the computer-side packet handler
void cph(packet);

// sends out a packet indicating a squib error
void badSquib(int, int, int, char);

// gets change in time
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

// determines if the next step in the program should start
bool nextStep();

// displays the controls
void help();

// glut callbacks & other stuff
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void keyReleased(unsigned char key, int x_pos, int y_pos);
void keySpecialReleased(int key, int x_pos, int y_pos);
void keyOps();
void processSpecialKeys(int key, int xx, int yy);
void specialKeyOps();
void menu(int id);
void display(void);

// rendering functions for the different parts
void renderFirebox(int, int);
void renderLunchbox(int, int);
void renderSquib(int, int, bool);

// main program
int main(int argc, char **argv)
{
	intro();

	init(argc, argv);

	// display the help menu here so the user can see it at least once
	help();

	// create the menu and attach the components
	glutCreateMenu(menu);
	glutAddMenuEntry("Quit", 1);
	glutAddMenuEntry("Toggle Pause", 2);
	glutAddMenuEntry("Assigning Addresses", 3);
	glutAddMenuEntry("Arm Random Squib", 4);
	glutAddMenuEntry("Arm Specific Squib", 5);
	glutAddMenuEntry("Fire Armed Squibs", 6);
	glutAddMenuEntry("Display Time Step in Terminal", 7);
	glutAddMenuEntry("Help", 8);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(display);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
	glutKeyboardUpFunc(keyReleased);// Called when a key is released
	glutSpecialUpFunc(keySpecialReleased);// Called when a special key is released
	glutSpecialFunc(processSpecialKeys);

    t1 = std::chrono::high_resolution_clock::now();
    glutMainLoop(); // starts the glut loop

	cout << "End of Simulation" << endl << endl;
	return 0;
}

// updates everything such as responding to key presses, updating display, and handling packets
void update()
{
	// check for key presses
	keyOps();

	// update the display
	display();

	if(!pause && nextStep())
	{
		if(mode == 'n') // do nothing mode
		{

		}
		else if(mode == 't') // keeps printing the time step
		{
			cout << timeStep << endl;
		}		
		else if(mode == 'r') // fire random squib mode
		{
			armRandomSquib();
		}
		else if(mode == 'a') // arms an individual squib
		{
			armIndividualSquib();
		}
		else if(mode == 'f') // fires all armed squibs
		{
			fireArmedSquibs();
		}

		// send out one packet from the computer
		// and let the fireboxes handle it
		if(!cQueue.empty())
		{
			fph(cQueue.front());
			cQueue.pop();
		}

		// send out one packet from the fireboxes
		// and let the computer handle it
		if(!fQueue.empty())
		{
			cph(fQueue.front());
			fQueue.pop();
		}

		// decrement firing squibs
		for(unsigned int i = 0; i < firingSquibs.size(); i++)
		{
			firingSquibs[i].fireCount--;
		}


		// removes all squibs that have finished firing so they wont be displayed as firing anymore
		bool noneRemoved = false;
		while(!noneRemoved)
		{
			noneRemoved = true;
				
			// remove expired Squibs
			for(unsigned int i = 0; i < firingSquibs.size(); i++)
			{
				if(firingSquibs[i].fireCount == 0)
				{
					universe.stopFire(firingSquibs[i].fPos, firingSquibs[i].lPos, firingSquibs[i].sPos);
					firingSquibs.erase(firingSquibs.begin() + i);
					noneRemoved = false;
				}
			}
		}

		timeStep++; // update the timeStep by 1 each time it loops
	}
}

// called when user reshapes window
void reshape(int n_w, int n_h)
{
	glViewport(0, 0, (GLsizei)n_w, (GLsizei)n_h);  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat)n_w / (GLfloat)n_h, 1.0, 100.0); 
	glMatrixMode(GL_MODELVIEW);
}

// simple intro to program
void intro()
{
	cout << endl << "Starting eSquib Protocol Simulation" << endl;
	cout << "Simulates DSSP's pyrotechnic system" << endl;
	cout << "dsspropulsion.com/pyrotechnics" << endl;
	cout << "created by Vance Piscitelli" << endl << endl;
}

// initializes the universe and the rest of the simulation
void init(int argc, char **argv)
{
	int numFireboxes = 0;
	int numLunchboxes = 0;
	int numSquibs = 0;

	cout << "Initializing Simulation" << endl;

	// initialize random
	srand(time(NULL));

	// get number of fireboxes
	while(numFireboxes <= 0 || numFireboxes > 16)
	{
		cout << "Set number of Fireboxes for the Universe (1-16): ";
		cin >> numFireboxes;
	}

	// get number of lunchboxes
	while(numLunchboxes <= 0 || numLunchboxes > 12)
	{
		cout << "Set number of Lunchboxes per Firebox (1-12)    : ";
		cin >> numLunchboxes;
	}

	// get number of squibs
	while(numSquibs <= 0 || numSquibs > 12)
	{
		cout << "Set number of Squibs per Lunchbox (1-8)        : ";
		cin >> numSquibs;
	}

	cout << "Initializing Universe with:" << endl;

	universe = Universe(numFireboxes, numLunchboxes, numSquibs, true);
	cout << "Fireboxes	: " << universe.getNumFireboxes() << endl;
	cout << "Lunchboxes	: " << universe.getNumLunchboxes() << endl;
	cout << "Squibs		: " << universe.getNumSquibs()<< endl;

	cout << "Universe Initialized" << endl << endl;

	// display universe in terminal
	universe.display();
	
	// initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE);
    glutInitWindowSize(w, h);
	glutInitWindowPosition(100, 100);
    glutCreateWindow("eSquib Protocol Simulation by Vance Piscitelli");
}

// arms an individual squib as specified by the user
void armIndividualSquib()
{
	int fireboxNum = -1;
	int lunchboxNum = -1;
	int squibNum = -1;

	cout << "Select Squib to Arm:" << endl;
	cout << "Note: Type Value in Terminal" << endl;

	// get the firebox
	while(fireboxNum < 0 || fireboxNum >= 16)
	{
		cout << "Firebox (0-15): ";
		cin >> fireboxNum;
	}

	// get the lunchbox
	while(lunchboxNum < 0 || lunchboxNum >= 12)
	{
		cout << "Lunchbox (0-11): ";
		cin >> lunchboxNum;
	}

	// get the squib
	while(squibNum < 0 || squibNum >= 8)
	{
		cout << "Squib (0-7): ";
		cin >> squibNum;
	}

	// sends the arm command to be made into a packet
	sendArm(fireboxNum, lunchboxNum, squibNum);

	// change mode back to default
	mode = 'n';
}

// sends a packet to arm a randomly selected squib
void armRandomSquib()
{

	// randomly selects one of the squibs
	int randomFirebox = rand() % universe.getNumFireboxes();
	int randomLunchbox = rand() % universe.getNumLunchboxes();
	int randomSquib = rand() % universe.getNumSquibs();

	cout << randomFirebox << randomLunchbox << randomSquib << endl;

	// sends the arm command to be made into a packet
	sendArm(randomFirebox, randomLunchbox, randomSquib);

	// change mode back to default
	mode = 'n';
}

// sends out an arm packet to arm specified squib
void sendArm(int fireboxNum, int lunchboxNum, int squibNum)
{
	packet armPacket;

	// set the values of the packet
	armPacket.type = 'a';
	armPacket.fbAdd = universe.getFireboxAddress(fireboxNum);
	armPacket.lbAdd = universe.getLunchboxAddress(fireboxNum, lunchboxNum);
	armPacket.sbAdd = universe.getSquibAddress(fireboxNum, lunchboxNum, squibNum);
	armPacket.bright = 4;

	// add the packet to the queue
	cQueue.push(armPacket);
}

// sends packet to fire all of the currently armed squibs
void fireArmedSquibs()
{
	packet firePacket;

	// set the values of the packet
	firePacket.type = 'f';
	firePacket.numFb = universe.getNumFireboxes();
	firePacket.numLb = universe.getNumLunchboxes();
	firePacket.numSb = universe.getNumSquibs();

	// add the packet to the queue
	cQueue.push(firePacket);

	// resets mode to default
	mode = 'n';
}


// normally, the addresses would be manually set via DIP switches on the back of the
// physical fireboxes. For this simulation, this function will just set the addresses.
void assignAddresses()
{
	int numFireboxes = universe.getNumFireboxes();
	int numLunchboxes = universe.getNumLunchboxes();
	int numSquibs = universe.getNumSquibs();

	packet addFb;

	// creates packet that assigns addresses
	addFb.type = 'u';
	addFb.numFb = numFireboxes;
	addFb.numLb = numLunchboxes;
	addFb.numSb = numSquibs;

	// for each firebox
	for(int i = 0; i < numFireboxes; i++)
	{		
		addFb.fbAdd = i;
		cQueue.push(addFb); // push packet into queue
	}
	
}

// the firebox-side packet handler
void fph(packet pack)
{
	static int address = 0;

	if(pack.type == 'u') // a universe setup packet
	{
		for(int i = 0; i < pack.numFb; i++)
		{
			universe.setFireboxAddress(i, address); // sets firebox address
			address++;

			for(int j = 0; j < pack.numLb; j++)
			{
				universe.setLunchboxAddress(i, j, address); // sets lunchbox address
				address++;

				for(int k = 0; k < pack.numSb; k++)
				{
					universe.setSquibAddress(i, j, k, address); // sets squib address
					address++;
				}
			}
		}
	}
	else if(pack.type == 'f') // a fire packet. fires all armed & active squibs
	{
		fSquib temp;

		for(int i = 0; i < 16; i++)
		{
			for(int j = 0; j < 12; j++)
			{
				for(int k = 0; k < 8; k++)
				{
					// squib will fire iff armed & active
					universe.fireSquib(i, j, k);

					if(universe.getFiring(i, j ,k))
					{
						temp.fPos = i;
						temp.lPos = j;
						temp.sPos = k;

						firingSquibs.push_back(temp); // add firing squib to vector to keep display updated
					}
				}
			}
		}
	}
	else if(pack.type == 'a') // arm packet
	{
		// if the specific squib is active
		if(universe.getActiveSquib(pack.fbAdd, pack.lbAdd, pack.sbAdd))
		{
			// arm specified squib
			universe.setArmedSquib(pack.fbAdd, pack.lbAdd, pack.sbAdd, true);
		
			// if squib not armed, send back an error packet
			if(!universe.getArmedSquib(pack.fbAdd, pack.lbAdd, pack.sbAdd))
			{
				badSquib(pack.fbAdd, pack.lbAdd, pack.sbAdd, 'u');
			}
		}
	}

}

// the computer-side packet handler
void cph(packet pack)
{
	if(pack.type == 'e') // an error packet
	{
		cout << "Error at Squib (" 	<< pack.fbAdd << ", " 
									<< pack.lbAdd << ", "
									<< pack.sbAdd << "). " << endl;
		if(pack.error == 'u') // problem arming squib
		{
			cout << "Could Not Arm Squib." << endl;
		}
	}
}

// creates a packet saying that the squib is bad for whatever reason
void badSquib(int fAdd, int lAdd, int sAdd, char err)
{
	packet badSb;

	badSb.type = 'e';
	badSb.fbAdd = fAdd;
	badSb.lbAdd = lAdd;
	badSb.sbAdd = sAdd;
	badSb.error = err; 

	fQueue.push(badSb);
}

// returns the change in time;
float getDT()
{
	float ret = 0;

	t2 = std::chrono::high_resolution_clock::now();
	ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
	t1 = std::chrono::high_resolution_clock::now();

	return ret;
}

// continues the simulation loop into the next step if enough time has passed
bool nextStep()
{
	static float timeNow = 0;

	timeNow += getDT();
	
	if(timeNow >= simSpeed)
	{
		timeNow = 0;
	
		return true; // allows next step to occur
	}
	
	return false; // prohibits next step
}

// prints out the help menu in the terminal
void help()
{
	cout << endl << "Simulation Controls:" << endl;
	cout << "P  - Pause Simulation" << endl;
	cout << "S  - Setup Universe (Assign Addresses)" << endl;
	cout << "R  - Random Arm Mode" << endl;
	cout << "A  - Arms Specific Squib Mode" << endl;
	cout << "F  - Fires All Armed Squibs" << endl;
	cout << "T  - Shows the Time Step in Terminal" << endl;
	cout << "D  - Display Universe in Terminal" << endl;
	cout << "H  - Display This in Terminal" << endl;	
	cout << "Note: The other window must be clicked on to input commands" << endl;
}

// keyboard inputs used by keyOps
void keyboard(unsigned char key, int x_pos, int y_pos)
{
	keyStates[key] = true;
}

// called when a key is released
void keyReleased(unsigned char key, int x_pos, int y_pos)
{
	keyStates[key] = false;
}

// called when a special key is released
void keySpecialReleased(int key, int x_pos, int y_pos)
{
	keySpecialStates[key] = false;
}

// handles when normal keys are pressed
void keyOps()
{
    // Handle keyboard input
    if(keyStates[27])//ESC
    {
        exit(0);
    }

	if(keyStates['h'] || keyStates ['H'])
	{
		help();
		keyStates['h'] = keyStates['H'] = false;
	}

	if(keyStates['d'] || keyStates ['D'])
	{
		universe.display();
		keyStates['d'] = keyStates['D'] = false;
	}
	if(keyStates['f'] || keyStates['F'])
	{
		cout << "Firing All Armed Squibs" << endl;
		mode = 'f';
		keyStates['f'] = keyStates['F'] = false;
	}

	if(keyStates['a'] || keyStates['A'])
	{
		cout << "Arm Specific Squib Mode Selected" << endl;
		mode = 'a';
		keyStates['a'] = keyStates['A'] = false;
	}

	if(keyStates['r'] || keyStates['R'])
	{
		cout << "Random Arm Mode Selected" << endl;
		mode = 'r';
		keyStates['r'] = keyStates['R'] = false;
	}

	if(keyStates['t'] || keyStates['T'])
	{
		cout << "Displaying Time Step in Terminal" << endl;
		mode = 't';
		keyStates['t'] = keyStates['T'] = false;
	}

	if(keyStates['p'] || keyStates['P'])
	{
		pause = !pause;
		
		if(pause)
		{
			cout << "Simulation Paused" << endl;
		}
		else
		{
			cout << "Simulation Unpaused" << endl;	
		}

		keyStates['p'] = keyStates['P'] = false;
	}

	if(keyStates['s'] || keyStates['S'])
	{
		cout << "Assigning Addresses to Current Universe" << endl;

		assignAddresses();

		keyStates['s'] = keyStates['S'] = false;
	}
}

// handles when special keys are pressed
void processSpecialKeys(int key, int xx, int yy)
{
	keySpecialStates[key] = true;
}

// for special keys such as the arrow keys
void specialKeyOps()
{
	// currently unused
}

// the right click menu options
void menu(int id)
{
	switch(id)
	{
		case 1:
			exit(0);
			break;
		case 2:
			pause = !pause;
			if(pause)
			{
				cout << "Simulation Paused" << endl;
			}
			else
			{
				cout << "Simulation Unpaused" << endl;	
			}
			break;
		case 3:
			cout << "Assigning Addresses to Current Universe" << endl;
			assignAddresses();
			break;
		case 4:
			cout << "Random Arm Mode Selected" << endl;
			mode = 'r';
			break;
		case 5:
			cout << "Arm Specific Squib Mode Selected" << endl;
			mode = 'a';
			break;
		case 6:
			cout << "Firing All Armed Squibs" << endl;
			mode = 'f';
			break;
		case 7:
			cout << "Displaying Time Step in Terminal" << endl;
			mode = 't';
			break;
		case 8:
			help();
			break;
	}
}

// renders a firebox
void renderFirebox(float x, float y)
{
	glColor3f(0.8f, 0.3f, 0.3f);
	glBegin(GL_QUADS);

	glVertex3f(x, y, 0.0f);
	glVertex3f(x, y + 0.2, 0.0f);
	glVertex3f(x + 0.2, y + 0.2, 0.0f);
	glVertex3f(x + 0.2, y, 0.0f);

	glEnd();
}

// renders a lunchbox
void renderLunchbox(float x, float y)
{
	glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_QUADS);

	glVertex3f(x, y, 0.0f);
	glVertex3f(x, y + 0.1, 0.0f);
	glVertex3f(x + 0.6, y + 0.1, 0.0f);
	glVertex3f(x + 0.6, y, 0.0f);

	glEnd();
}

// renders a squib
void renderSquib(float x, float y, bool firing)
{
	if(firing)
	{
		glColor3f(0.9f, 0.7f, 0.0f);
	}
	else
	{
		glColor3f(0.3f, 0.1f, 0.0f);
	}

	glBegin(GL_QUADS);

	glVertex3f(x, y, 0.0f);
	glVertex3f(x, y + 0.05, 0.0f);
	glVertex3f(x + 0.06, y + 0.05, 0.0f);
	glVertex3f(x + 0.06, y, 0.0f);

	glEnd();
}

void display(void)
{
	glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // clear the background to this color
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	glTranslatef(-4.2f, 2.8f, -5.0f); // to stop shape from getting culled

	float x = 0.0f;
	float xS;
	float y = 0.0f;
	
	for(int i = 0; i < universe.getNumFireboxes(); i++)
	{
		y -= 0.3;
		renderFirebox(x, y);

		x += 0.3;
		for(int j = 0; j < universe.getNumLunchboxes(); j++)
		{

			renderLunchbox(x, y + 0.1);

			xS = x + 0.01;
			for(int k = 0; k < universe.getNumSquibs(); k++)
			{
				
				renderSquib(xS, y + 0.125, universe.getFiring(i, j, k));
				xS += 0.0725;
			}

			x += 0.68;
		}

		x = 0.0; // reset x for next firebox & lunchboxes
	}

	glFlush();
}

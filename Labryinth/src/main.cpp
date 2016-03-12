/*
	@author: Zeeshan Sajid, Vance Piscitelli 
	@brief: This program allows the user to play a game called Labyrinth utilizing openGL 
	This program loads models using a function called objLoader, in which it also adds uv textures 
	for the model and additionally returning a trimesh for Bullet in order to create static shapes for collision. 
	From there, we loaded 15x15 matrix of cubes which we used as our board after reading it in from a file. The 
	file had information such as where the hole existed and if it was static and so on. Essentially
	each and every cube had its own collision, texture, and essentially it was used together to create a much more 
	dynamic board than what could be received before.

	Functionalties:
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

	Irrklang audio library:
		Overview:
		Irrklang is a cross platform sound library for C++, C# and all .NET languages, essentially,
	it is free for non-commercial, while there si a pro version that comes with more features than the free.
	Since this is not offered on GitHub it is much harder to attain it than by just doing sudo apt-get install lib
	
	Instructions for usage:
		1. To download, go to http://www.ambiera.com/irrklang/, and click download and download the zip file 
		Irrklang has available, and it will come in as a zip file
		2. Extract the zip file and run the hello world or any program to see if it works and is not corrupted
		3. From there, extract the folder specifically catered to your OS. Since this class is using Linux, grab
		linux-gcc-64, along with the include folder which will host most of the header files
		4. From there go into your makefile and add this line or equivalent file location:
		OPTS = -I"../src/include"  ../src/linux-gcc-64/libIrrKlang.so -pthread
		5. From there, add it to compiler flag just like you would with LIBS, something along the lines of this:
		$(CC) $(CXXFLAGS) ../src/shader.cpp ../src/main.cpp -o ../bin/Bullet $(LDFLAGS) $(LIBS) $(OPTS)
		6. Within your .cpp file or wherever you are running it, #include<irrKlang.h> and if you want to use the namespace you can just by doing using namespace irrKlang
		7. From there, the program should be compilable and you can start using irrKlang for a lot of different sound libraries.
*/

/*
shader.h contains <GL/glew.h> <GL/glut.h>  <iostream> <fstream> <string.h>
*/
#define GLM_FORCE_RADIANS 
#include <btBulletDynamicsCommon.h>
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
#include "shader.h"
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/color4.h>
#include <Magick++.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <ctime>
#include <math.h>
#include <irrKlang.h> // This is the sound library
using namespace irrklang; // This is so I don't have to use scope resolution operator each time for soundEngine

#define BIT(x) (1<<(x))
// Different collision objects
enum collisiontypes
{
    COL_NOTHING = 0, // Collide with nothing
    COL_PLANE = BIT(0), // the plane
    COL_CUBE = BIT(1), // the cube
    COL_SPH = BIT(2), // the ball 
	COL_FRAME = BIT(3), // The frame that encapsulates the overall labyrinth
};

// Collision enumerations
int sphereCollision  = COL_CUBE|  COL_PLANE | COL_FRAME | COL_SPH;
int cubeCollision = COL_SPH;
int planeCollision = COL_SPH;
int frameCollision = COL_SPH;

// Global sound stuff
ISoundEngine *SoundEngine = createIrrKlangDevice();

//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
	GLfloat position[3]; // Holds the position
	GLfloat normals[3]; // Holds the normals
	GLfloat uv[2]; // Holds the uvs
};

// The main class for the cubes
struct bCube
{
	// States the height of the cube in real world 
	float height = 0;
	// How high the cube needs to be
	float targetHeight = 0;
	// The type, s is for static, h is for a cube with a hole, b is intiial type value for debugging if read in incorrectly
	char type = 'b'; // b for bug
};

// Stores each game for the user
struct game
{
	// The name of the user
	string winner;
	int level; // This is the level the player played on
	int hours; // Amount of hours played
	int minutes; // Amount of minutes played
	int seconds; // Amount of seconds played
	int mseconds; // Amount of miliseconds played
};

// Comparator function
bool compareByScore(game a, game b)
{
	// Comparing the users hours, minutes and seconds, and mseconds and returning the result to vecot sort function
	return ((a.hours <= b.hours ) && (a.minutes <= b.minutes) && (a.seconds <= b.seconds) && (a.mseconds <= b.mseconds));
}

// Vector of top score scored by player
vector<game> topScores;

// Lighting globals
int ambOn = 1; // for ambient lighting
int disOn = 0; // For directional lighting
int spotOn = 0; // For spot lighting
int pointOn = 1; // For point Lights

// Constants for the overall board
const int cubesW = 15; // The number of cubes horizontally
const int cubesH = 15; // The number of cubes vertically
const int numCubes = cubesW * cubesH; // The overall number of cubes in the game
const int numModels = 4; // The number of models within the program, should be frame, a cube, a cube with a hole, along with sphere

// The type of themes within the game
bool goldenAge = false;
bool stylishWood = true;

// A contigous array of bCube objects to store data from the array
bCube board[cubesW][cubesH];

// This is for magick
GLuint *texture = NULL; // For texture, need to dynamically allocate
Magick::Blob *bigBlob = NULL;;

// This is the number of vertices that belong to each of the different objects
int numVert[numModels]; // holds verts for frame, spheres, bounding box
int cubeVert = 0; // Holds the number of vert for the cubes
int holeVert = 0; // Holds the number of vert for cubes with holes

std::vector< glm::vec3 > vertices; // Holds the number of positional verticies from the file
std::vector< glm::vec2 > uvs; // Holds the uv verticies
std::vector< glm::vec3 > normals; // Holds the normals

// Global variables
int w = 1080, h = 720;// window size
GLuint program;// the GLSL program handle
GLuint vbo_geometry[numModels + 2];// VBO handle for our geometry
bool* keyStates = new bool[256]; // whether or not each key is pressed
bool* keySpecialStates = new bool[256]; // for the special keys

// Title screen
int titleScreen = 1; // This will be used to tell if the game is on the title screen
int showTopScore = 0; // This will be used to tell if the user wants to see the top scores

// Timer
int hours = 0;
int minutes = 0;
int seconds = 0;
int mseconds = 0;

// name of player
string player1Name;

// uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader
GLint loc_m;
GLint loc_view;

// attribute locations
GLint loc_position;
GLint loc_uv;
GLint loc_normal;

// Transform matrices
glm::mat4 model[numModels];//obj->world each object should have its own model matrix
glm::mat4 cubes[cubesH][cubesW]; // holds the 15x15 array of cube matrices
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection

glm::vec3 cameraPos;// The position of the camera
glm::vec3 focusPos;// Where to have the camera focus
glm::vec3 cameraAngle;// The camera angle to look at the board

// What model to focus with the camera. This program just focuses on ball unlike previous
int modelToFocus = 0;

// Pauses the game
bool pauseGame = false;

// The transitioning state variable when moving from one level to another
bool transitioning = false;

// The state variable to make the game difficult
bool difficult = false;

// The number of balls in the game (currently only handles a max of 2)
int numSpheres = 1;

// The stage the game starts out with
int currentLevel = 1;

// This stuff is for bullet
btBroadphaseInterface *broadphase = NULL; // Build the broadphase
btDefaultCollisionConfiguration* collisionConfiguration = NULL;// Set up the collision
btCollisionDispatcher* dispatcher = NULL; // Set up the dispatcher
btSequentialImpulseConstraintSolver* solver = NULL; // The actual physics solver
btDiscreteDynamicsWorld* dynamicsWorld = NULL; // The world.

// The rigid bodies that will be used for bullet
btRigidBody* planeRigidBody = NULL;
btRigidBody* sphereRb = NULL;
btRigidBody* sphere2Rb = NULL;
btRigidBody* cubeRb[cubesW * cubesH];
btRigidBody* frameRb = NULL;
btRigidBody* boxRb = NULL; // bounding box

// TriangleMeshes
btTriangleMesh *sphereMesh = NULL;
btTriangleMesh *sphere2Mesh = NULL;
btTriangleMesh *cubeMesh = NULL;
btTriangleMesh *holeMesh = NULL;
btTriangleMesh *frameMesh = NULL;
btTriangleMesh *boxMesh = NULL;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void keyReleased(unsigned char key, int x_pos, int y_pos);
void keySpecialReleased(int key, int x_pos, int y_pos);
void keyOps();
void processSpecialKeys(int key, int xx, int yy);
void specialKeyOps();
void moveSphere(int, int);

// sets the velocity for a paddle, making sure its within valid range.
void addVelocity(float, float, float);

//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

// menu options/ controls
void menu(int id);

// loaders
Vertex *objLoader(string imageFile, int &numVertices, btTriangleMesh *& sendBackMesh);

// updates the rigid bodies. called in update()
void updateRb(btRigidBody*, int);

// create a image object and then have it read it, create textures bind it, and then do other gibberish things
void loadImage(string, int);

// loads a level from the specified text file. 2nd param true if you want it to load immediately.
void loadLevel(string, bool);

// updates the camera angle to simulate board tilting
void updateCameraAngle();

// resets the ball's position
bool sendBallToStart();

// This reads in the scores from the .txt file most likely into a map
bool readInTopScores();

/*
	Writes to topSCores in a file, essentially used to record games won by player x
	@param: NULL
	@pre: Nothing, if no topScores, it writes out a file topScores
	@post: Reads out topScores
*/
void writeToTopScores();

// called while board is changing between 2 levels
void transition();

// gets distance between a cube and the sphere
float getDistToSphere(int i, int j);

//--Main
int main(int argc, char **argv)
{
	// Initialize Magick++
	Magick :: InitializeMagick(*argv);
	
	// Initialize random
	// can be used to set the cubes to a random height
	srand(time(NULL));

	// If the arguments passed in when running bullet is greater than two, record the names
	if(argc >1)
	{
		// Use the names passed in from the parameter
		player1Name = argv[1];
	}

	// Otherwise default the players name
	else
	{
		player1Name = "Player1";	
	}

	// Read in top scores from file if it exist
	readInTopScores();

    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    glutCreateWindow("PA11 - Labyrinth by Vance Piscitelli & Zeeshan Sajid");

	// Create the menu and attach the components
	glutCreateMenu(menu);
	glutAddMenuEntry("Start", 1);
	glutAddMenuEntry("Stop", 2);
	glutAddMenuEntry("Quit", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
    {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
    }

    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
	glutKeyboardUpFunc(keyReleased);// Called when a key is released
	glutSpecialUpFunc(keySpecialReleased);// Called when a special key is released
	glutSpecialFunc(processSpecialKeys);

	// Play a starting song for the game
	SoundEngine->play2D("./breakout.wav", true);

	// Initialize all of our resources(shaders, geometry)	
	if(initialize())
    {
        t1 = std::chrono::high_resolution_clock::now();

        glutMainLoop();
	}

	cleanUp();
	return 0;
}

//--Implementations
void render()
{
    //--Render the scene

    // clear the screen
    glClearColor(0.05, 0.05, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// show the title screen
    if(titleScreen == 1 && showTopScore == 0)
    {
	  	string start = "Press 'Q' to start game";
	  	glColor3f(1.0, 0.0, 0.0); 
	  	glRasterPos3f(-.3,.0,0);

		// If so render these two things onto the screen
		for(int i = 0; i < start.size(); i++)
		{    
	    	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, start[i]);
		}

		// Rinse and repeat
		start = "Press '9' for top scores";
		glColor3f(1.0, 0.0, 0.0); 
		glRasterPos3f(-.3,-0.2,0);

	  	for(int i = 0; i < start.size(); i++)
		{    
	        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, start[i]);
		}
		pauseGame = 1;
    }
    else if(showTopScore == 1 && titleScreen == 0)// shows the top scores
    {
    	// Create a buffer for the instructions
		char instructions[60];

		// Decide the color and location for the rasterization process
		glColor3f(1.0, 0.0, 0.0); 
		glRasterPos3f(-.4,.9,0);

		// Place the string into the instructions buffer
		sprintf(instructions, "Top 10 Scores, based on matches won");

		// Loop through and print it out to the screen
		for(int i = 0; instructions[i] != '\0'; i++)
		{    
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, instructions[i]);
		}

		// Sort the vector of top scores
		std :: sort(topScores.begin(), topScores.end(), compareByScore);

			// This places the rasterization
			float yvalue = 0.8;

			// Rasterize the top scores based on the topSCores in the vector
			if(topScores.size() < 10)
			{
				for(int i = 0; i < topScores.size(); i++)
				{
					if(!topScores[i].winner.empty() )
					{
						char buffer[90];
						sprintf(buffer, "%d. %s =  %d hours, %d minutes, %d seconds" , i+1,  topScores[i].winner.c_str(), topScores[i].hours, topScores[i].minutes, topScores[i].seconds);

						glColor4f(0,1,0, 1.0f);
						glRasterPos3f( -.4, yvalue, 0.0 );

						for(int i = 0; buffer[i] != '\0'; i++)
						{    
							glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, buffer[i]);
						}
						
						yvalue -= 0.08;
					}
				}
			}
			else if(topScores.size() > 10)// Rinse and repeat
			{
				for(int i = 0; i < 10; i++)
				{
					if(!topScores[i].winner.empty() )
					{
						char buffer[90];
						sprintf(buffer, "%d. %s =  %d hours, %d minutes, %d seconds" , i+1,  topScores[i].winner.c_str(), topScores[i].hours, topScores[i].minutes, topScores[i].seconds);

						glColor4f(0,1,0, 1.0f);
						glRasterPos3f( -.4, yvalue, 0.0 );

						for(int i = 0; buffer[i] != '\0'; i++)
						{    
							glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, buffer[i]);
						}
						yvalue -= 0.08;
					}
				}
			}

			// Render some more instructions to the screen
			char Leaveinstructions[30];
			glColor3f(1.0, 0.0, 0.0); 
		  	glRasterPos3f(-.4,-.9,0);

			sprintf(Leaveinstructions, "Press . key to leave");

			for(int i = 0; Leaveinstructions[i] != '\0'; i++)
			{    
				glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, Leaveinstructions[i]);
			}
    }
    else if(showTopScore == 0 && titleScreen == 0)
    {
		// To rasterize, need to turn off program
		glUseProgram(0);

		// Create a buffer to store the text that needs to be rasterized
		char buffer[70];

		// rasterization process, use the name as the string and loop through for both the instructions and name
		glColor4f(1.0,1,0, 1.0f);
		glRasterPos3f( -.1, .8, 0.0 );

		// Rasterize the hours, minutes, seconds, mseconds
		sprintf(buffer, "%s's time is: %d : %d : %d : %d", player1Name.c_str(), hours, minutes, seconds, mseconds);

		// Loop through and display to screen
		for(int i = 0; buffer[i] != '\0'; i++)
		{    
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, buffer[i]);
		}

		// draw the first sphere to the screen

		// premultiply the matrix for this example
		mvp = projection * view * model[0];

		// renable the shader program
		glUseProgram(program);

		// upload mvp matrix, along with the model and view matrix
		glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniformMatrix4fv(loc_m, 1, GL_FALSE, glm::value_ptr(model[0]));
		//glUniformMatrix4fv(loc_view, 1, GL_FALSE, glm::value_ptr(view));

		// set up the Vertex Buffer Object so it can be drawn
		glEnableVertexAttribArray(loc_position);
		glEnableVertexAttribArray(loc_uv);
		glEnableVertexAttribArray(loc_normal);

		// cube is in 0 so start after that
		glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[1]);

		// set pointers into the vbo for each of the attributes(position and color)
	   	glVertexAttribPointer(loc_position,//location of attribute
					          3,//number of elements
					          GL_FLOAT,//type
					          GL_FALSE,//normalized?
					          sizeof(Vertex),//stride
					          0);//offset

	   	
	   	if(stylishWood) // If stylish theme, then use different texture
	   	{
			glBindTexture(GL_TEXTURE_2D, texture[1]);
		}
		else if(goldenAge)// If goldenAge, then use different texture
	   	{
			glBindTexture(GL_TEXTURE_2D, texture[1]);
		}

		// Get the information for the uv textures to pass into the vertex shader
	   	glVertexAttribPointer(loc_uv,
					          2,
					          GL_FLOAT,
					          GL_FALSE,
					          sizeof(Vertex),
					          (void*)offsetof(Vertex,uv));

	   	// Draw the sphere onto the screen with triangles
		glDrawArrays(GL_TRIANGLES, 0, numVert[0]);//mode, starting index, count

		// draw the second sphere, rise and repeat
		if(numSpheres == 2)
		{
			// premultiply the matrix for this example
			mvp = projection * view * model[2];

			// enable the shader program
			glUseProgram(program);

			// upload the matrix to the shader
			glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

			glUniformMatrix4fv(loc_m, 1, GL_FALSE, glm::value_ptr(model[2]));

			// set up the Vertex Buffer Object so it can be drawn
			glEnableVertexAttribArray(loc_position);
			glEnableVertexAttribArray(loc_uv);
			glEnableVertexAttribArray(loc_normal);

			// cube is in 0 so start after that
			glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[4]);

			// set pointers into the vbo for each of the attributes(position and color)
		   	glVertexAttribPointer(loc_position,//location of attribute
							      3,//number of elements
							      GL_FLOAT,//type
							      GL_FALSE,//normalized?
							      sizeof(Vertex),//stride
							      0);//offset

			glBindTexture(GL_TEXTURE_2D, texture[6]);

		   	glVertexAttribPointer(loc_uv,
							      2,
							      GL_FLOAT,
							      GL_FALSE,
							      sizeof(Vertex),
							      (void*)offsetof(Vertex,uv));


			glDrawArrays(GL_TRIANGLES, 0, numVert[2]);//mode, starting index, count
		}

		// draw the frame
	
		// premultiply the matrix for this example
		mvp = projection * view * model[1];

		// enable the shader program
		glUseProgram(program);

		// upload the matrix to the shader
		glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

		glUniformMatrix4fv(loc_m, 1, GL_FALSE, glm::value_ptr(model[1]));

		// set up the Vertex Buffer Object so it can be drawn
		glEnableVertexAttribArray(loc_position);
		glEnableVertexAttribArray(loc_uv);
		glEnableVertexAttribArray(loc_normal);

		// cube is in 0 so start after that
		glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[2]);

		// set pointers into the vbo for each of the attributes(position and color)
	   	glVertexAttribPointer(loc_position,//location of attribute
						      3,//number of elements
						      GL_FLOAT,//type
						      GL_FALSE,//normalized?
						      sizeof(Vertex),//stride
						      0);//offset
	   	if(stylishWood)
	   	{
			glBindTexture(GL_TEXTURE_2D, texture[2]);
		}
		else if(goldenAge)
	   	{
			glBindTexture(GL_TEXTURE_2D, texture[7]);
		}

		// Get the information and place it into the vertex shader
	   	glVertexAttribPointer(loc_uv,
						      2,
						      GL_FLOAT,
						      GL_FALSE,
						      sizeof(Vertex),
						      (void*)offsetof(Vertex,uv));

	   	// Draw the frame onto the screen
		glDrawArrays(GL_TRIANGLES, 0, numVert[1]);// mode, starting index, count

		// loop through the number of cubes
		for(int j = 0; j < cubesH; j++)
		{
			for(int k = 0; k < cubesW; k++)
			{
				// If the cubes are s, e, or b, then check for the different difficulty options and place them onto the screen differently
				if(board[j][k].type == 's' || board[j][k].type == 'b' || board[j][k].type == 'e')
				{
					cubes[j][k][3][0] = j * 2; // x pos
					cubes[j][k][3][2] = k * 2; // z pos

					// If c is not pressed then change the y position to be more obivious
					if(!difficult)
					{
						cubes[j][k][3][1] = board[j][k].height; // y pos
					}

					// Otherwise, change the distance based on the sphere coming closer to said cube
					else
					{
						float dist = getDistToSphere(j,k);

						if(dist < 3)
						{
							cubes[j][k][3][1] = board[j][k].height;
						}
						else
						{
							cubes[j][k][3][1] = board[j][k].height * (3 / dist);
						}
					}

					// premultiply the matrix for this example
					mvp = projection * view * cubes[j][k];

					// enable the shader program
					glUseProgram(program);

					// upload the matrix to the shader
					glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

					glUniformMatrix4fv(loc_m, 1, GL_FALSE, glm::value_ptr(cubes[j][k]));

					// set up the Vertex Buffer Object so it can be drawn
					glEnableVertexAttribArray(loc_position);
					glEnableVertexAttribArray(loc_uv);
					glEnableVertexAttribArray(loc_normal);

					glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[0]);

					// set pointers into the vbo for each of the attributes(position and color)
				   	glVertexAttribPointer(loc_position,//location of attribute
										  3,//number of elements
										  GL_FLOAT,//type
										  GL_FALSE,//normalized?
										  sizeof(Vertex),//stride
										  0);//offset

				   	// Check to see if wood style, then apply said texture
				   	if(stylishWood)
				   	{
						if(board[j][k].type == 's')
						{
							glBindTexture(GL_TEXTURE_2D, texture[0]);
						}
						else if(board[j][k].type == 'b')
						{
							glBindTexture(GL_TEXTURE_2D, texture[4]);
						}
						else if(board[j][k].type == 'e')
						{
							glBindTexture(GL_TEXTURE_2D, texture[5]);
						}
					}
				   	else if(goldenAge)// Otherwise check for goldenAge and do the same
				   	{
						if(board[j][k].type == 's')
						{
							glBindTexture(GL_TEXTURE_2D, texture[8]);
						}
						else if(board[j][k].type == 'b')
						{
							glBindTexture(GL_TEXTURE_2D, texture[4]);
						}
						else if(board[j][k].type == 'e')
						{
							glBindTexture(GL_TEXTURE_2D, texture[5]);
						}
					}

				   	glVertexAttribPointer(loc_uv,
										  2,
										  GL_FLOAT,
										  GL_FALSE,
										  sizeof(Vertex),
										  (void*)offsetof(Vertex,uv));
/*
				   	glVertexAttribPointer(loc_normal, 
				   							3,
				   							GL_FLOAT,
				   							GL_FALSE,
				   							sizeof(Vertex),
*/
					glDrawArrays(GL_TRIANGLES, 0, cubeVert);//mode, starting index, count
				}

				// check to se eif itis a hole
				else if(board[j][k].type == 'h')
				{
					cubes[j][k][3][0] = j * 2;
					cubes[j][k][3][2] = k * 2;

					// Do the same thing as above with the difficulty but use holes
					if(!difficult)
					{
						cubes[j][k][3][1] = board[j][k].height; // y pos
					}
					else
					{
						float dist = getDistToSphere(j,k);

						if(dist < 3)
						{
							cubes[j][k][3][1] = board[j][k].height;
						}
						else
						{
							cubes[j][k][3][1] = board[j][k].height * (3 / dist);
						}
					}

					// premultiply the matrix for this example
					mvp = projection * view * cubes[j][k];

					// enable the shader program
					glUseProgram(program);

					// upload the matrix to the shader
					glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

					glUniformMatrix4fv(loc_m, 1, GL_FALSE, glm::value_ptr(cubes[j][k]));

					// set up the Vertex Buffer Object so it can be drawn
					glEnableVertexAttribArray(loc_position);
					glEnableVertexAttribArray(loc_uv);
					glEnableVertexAttribArray(loc_normal);

					glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[3]);

					// set pointers into the vbo for each of the attributes(position and color)
				   	glVertexAttribPointer(loc_position,//location of attribute
										  3,//number of elements
										  GL_FLOAT,//type
										  GL_FALSE,//normalized?
										  sizeof(Vertex),//stride
										  0);//offset

				   	if(stylishWood)
				   	{
						glBindTexture(GL_TEXTURE_2D, texture[3]);
					}
					else if(goldenAge)
					{
						glBindTexture(GL_TEXTURE_2D, texture[9]);						
					}

				   	glVertexAttribPointer(loc_uv,
										  2,
										  GL_FLOAT,
										  GL_FALSE,
										  sizeof(Vertex),
										  (void*)offsetof(Vertex,uv));
/*
				   	glVertexAttribPointer(loc_normal, 
				   							3,
				   							GL_FLOAT,
				   							GL_FALSE,
				   							sizeof(Vertex),
				   							0);
*/
					glDrawArrays(GL_TRIANGLES, 0, holeVert);//mode, starting index, count
				}
			}
		}	
	}

	if(ambOn)
	{
		glUniform4f(glGetUniformLocation(program, "ambientLight"), 0.15, 0.15, 0.15, 1);
	}
	else
	{
		glUniform4f(glGetUniformLocation(program, "ambientLight"), 0.05, 0.05, 0.05, 1);
	}

	// directional light
	if(disOn)
	{
		glUniform3f(glGetUniformLocation(program ,"directionalLight.vDirection"),   2.0f, 1.0f, 2.0f);
		glUniform3f(glGetUniformLocation(program ,"directionalLight.ambient"), 0.01f, 0.01f, 0.01f);
		glUniform3f(glGetUniformLocation(program ,"directionalLight.diffuse"),   0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(program ,"directionalLight.specular"), 0.0f, 0.0f, 0.0f);
	}
	else
	{
		glUniform3f(glGetUniformLocation(program ,"directionalLight.vDirection"),   2.0f, 1.0f, 2.0f);
		glUniform3f(glGetUniformLocation(program ,"directionalLight.ambient"), 0.5f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(program ,"directionalLight.diffuse"),   0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(program ,"directionalLight.specular"), 0.0f, 0.0f, 0.0f);	
	}

	// spotlight
	if(spotOn == 1)
	{
		glUniform3f(glGetUniformLocation(program, "spotLight[0].position"), sphereRb->getCenterOfMassPosition().x(), 
																			sphereRb->getCenterOfMassPosition().y() + 12, 
																			sphereRb->getCenterOfMassPosition().z());
		glUniform3f(glGetUniformLocation(program, "spotLight[0].direction"), 0,-1,0);
		glUniform3f(glGetUniformLocation(program, "spotLight[0].ambient"), 16.0f, 16.0f, 16.0f);
		glUniform3f(glGetUniformLocation(program, "spotLight[0].diffuse"), 8.0f, 8.0f, 8.0f);
		glUniform3f(glGetUniformLocation(program, "spotLight[0].specular"), 8.0f, 8.0f, 8.0f);
		glUniform1f(glGetUniformLocation(program, "spotLight[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(program, "spotLight[0].linear"), 0.09);
		glUniform1f(glGetUniformLocation(program, "spotLight[0].quadratic"), 0.032);
		glUniform1f(glGetUniformLocation(program, "spotLight[0].innerCut"), glm::cos(glm::radians(4.5f)));
		glUniform1f(glGetUniformLocation(program, "spotLight[0].spotCosOuterCutOff"), glm::cos(glm::radians(16.0f)));
	}
	else // turn off spotlight
	{
		glUniform3f(glGetUniformLocation(program, "spotLight[0].position"), sphereRb->getCenterOfMassPosition().x(), 
																			sphereRb->getCenterOfMassPosition().y() + 12, 
																			sphereRb->getCenterOfMassPosition().z());
		glUniform3f(glGetUniformLocation(program, "spotLight[0].direction"), 0,-1,0);
		glUniform3f(glGetUniformLocation(program, "spotLight[0].ambient"), 0.0f, 00.0f, 0.0f);
		glUniform3f(glGetUniformLocation(program, "spotLight[0].diffuse"), 0.0f, 00.0f, 0.0f);
		glUniform3f(glGetUniformLocation(program, "spotLight[0].specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(program, "spotLight[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(program, "spotLight[0].linear"), 0.09);
		glUniform1f(glGetUniformLocation(program, "spotLight[0].quadratic"), 0.032);
		glUniform1f(glGetUniformLocation(program, "spotLight[0].innerCut"), glm::cos(glm::radians(4.5f)));
		glUniform1f(glGetUniformLocation(program, "spotLight[0].spotCosOuterCutOff"), glm::cos(glm::radians(7.0f)));
	}

	// pointlight
	if(pointOn)
	{
		// Make the point light look at a certain position to shine its light and calculate it based on the model
		glUniform3f(glGetUniformLocation(program, "pLight[0].position"), -2.0, 0.0, 0.0);

		// Do the ambient diffuse and specular lighting values for the point light
		glUniform4f(glGetUniformLocation(program, "pLight[0].ambient"), 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4f(glGetUniformLocation(program, "pLight[0].diffuse"), 10.0f, 10.0f, 10.0f, 1.0f);
		glUniform4f(glGetUniformLocation(program, "pLight[0].specular"), 10.0f, 10.0f, 10.0f, 1.0f);

		//Calculate intensities.
		glUniform1f(glGetUniformLocation(program, "pLight[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(program, "pLight[0].linear"), 0.09);
		glUniform1f(glGetUniformLocation(program, "pLight[0].quadratic"), 0.032);
	}
	else //Otherwise make it not noticeable
	{
		glUniform3f(glGetUniformLocation(program, "pLight[0].position"), -2.0, 0.0, 0.0);
		glUniform4f(glGetUniformLocation(program, "pLight[0].ambient"), 0.5f, 0.5f, 0.5f,1.0f);
		glUniform4f(glGetUniformLocation(program, "pLight[0].diffuse"), 2.4f, 2.4f, 2.4f, 1.0f);
		glUniform4f(glGetUniformLocation(program, "pLight[0].specular"), 0.0f, 0.0f, 0.0f, 1.0f);
		glUniform1f(glGetUniformLocation(program, "pLight[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(program, "pLight[0].linear"), 0.09);
		glUniform1f(glGetUniformLocation(program, "pLight[0].quadratic"), 0.032);
	}

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_uv);  
    glDisableVertexAttribArray(loc_normal); 
          
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
	// check for any key presses and handle them accordingly
	keyOps();
	specialKeyOps();

	// update camera tilt to simulate board tilting
	updateCameraAngle();

	// update positions if board is transitioning between levels
	transition();

	view = glm::lookAt( cameraPos, // eye Position
                    	focusPos, // focus point
                    	cameraAngle); // positive Y is up

    // total time
    float dt = getDT();// if you have anything moving, use dt.
    static float angle = 0.0;

	// time updating
	// If not false, then do
    if(!pauseGame)
    {
    	// Add up the different hours/ minutes/ seconds/ mseconds
	    mseconds+= 1;
	    if(hours == 24)
	    {
	    	hours = 0;
	    }

	    if(minutes == 60)
	    {
	    	hours += 1;
	    	minutes = 0;
	    }

	    if(seconds == 60)
	    {
	    	minutes +=1;
	    	seconds = 0;
	    }

	    if(mseconds == 60)
	    {
	    	seconds +=1;
	    	mseconds = 0;
	    }
	}

    if(pauseGame == true)
    {
    	dt = 0; // Don't let dt effect anything
    }
	
	angle += dt *M_PI/2;

    // update physics world
    dynamicsWorld->stepSimulation(dt, 10);

	// update the Rbs
	updateRb(sphereRb, 0);
	
	// Check to see if the second ball exist, if so then update the rigid body
	if(numSpheres == 2)
	{
		updateRb(sphere2Rb, 2);
	}

	// Update the frames rigidbody
	updateRb(frameRb, 1);

	// Loop through the number of cubes
	for(int j = 0; j < cubesH; j++)
	{
		for(int k = 0; k < cubesW; k++)
		{
			// this is for bullet
			btTransform trans;
			btScalar m[16];

			// Get the rigidbody to model for rendering
			cubeRb[j * cubesH + k]->getMotionState()->getWorldTransform(trans);
			trans.getOpenGLMatrix(m);

			cubes[j][k] = glm::make_mat4(m);

			// if it is the end cube
			if(board[j][k].type == 'e')
			{

				btVector3 spherePos = sphereRb->getCenterOfMassPosition();
				btVector3 goal = cubeRb[j * cubesH +k]->getCenterOfMassPosition();

				// Check to see if the ball is good enough to be considered at the end line
				if(	(spherePos.x() <= (goal.x()+0.7) && spherePos.x() >= (goal.x()-0.7)) &&  
					(spherePos.z() <= (goal.z()+0.7) && spherePos.z() >= (goal.z()-0.7)))
				{
					// then check if this is the only ball
					if(numSpheres != 2)
					{
						// If it is the only ball, then start doing the changing process
						transitioning = true;
						bool complete = false;

						// This is a level that changes all of the rigidbodies to the next level and back to the first upon completion
						if(currentLevel == 1)
						{
							currentLevel++;
							loadLevel("level2.txt", false);
							sendBallToStart();
						}
						else if(currentLevel == 2)
						{
							currentLevel++;
							loadLevel("level3.txt", false);
							sendBallToStart();
						}
						else if(currentLevel == 3)
						{
							currentLevel = 1; // change once more levels are created
							loadLevel("level1.txt", false);
							complete = true;
							sendBallToStart();
						}

						// If the level becomes complete then play a sound
						if(!complete)
						{
							SoundEngine->play2D("./smallCrowd.wav", false);
						}

						// If it's the third level then play a longer applausing noise 
						if(complete)
						{
							SoundEngine->play2D("./crowedApplause.wav", false);

							// Upon level 3 completion, then save the top score to the file
							game tmp;

							tmp.winner = player1Name;

							bool playerExist = false;
							for(int i = 0; i < topScores.size(); i++)
							{
								if(player1Name == topScores[i].winner)
								{
									playerExist = true;

									if((minutes <= topScores[i].minutes))
									{
										if(seconds <= topScores[i].seconds)
										{
											if(mseconds<= topScores[i].mseconds)
											{
												topScores[i].level = currentLevel;
												topScores[i].hours = hours;
												topScores[i].minutes = minutes;
												topScores[i].seconds = seconds;
												topScores[i].mseconds = mseconds;
											}
										}
									}
								}
							}

							if(!playerExist)
							{
								tmp.hours = hours;
								tmp.minutes = minutes;
								tmp.seconds = seconds;
								tmp.mseconds = mseconds;
								topScores.push_back(tmp);
							}

							hours = minutes = seconds = mseconds = 0;
						}
					}
					else // Otherwise, make sure the second ball is at the correct position, and rinse and repeat like the previous one
					{
						btVector3 sphere2Pos = sphere2Rb->getCenterOfMassPosition();

						if(	(sphere2Pos.x() <= (goal.x()+0.7) && sphere2Pos.x() >= (goal.x()-0.7)) &&  
						(sphere2Pos.z() <= (goal.z()+0.7) && sphere2Pos.z() >= (goal.z()-0.7)))
							{
							transitioning = true;
							bool complete = false;

							if(currentLevel == 1)
							{
								currentLevel++;
								loadLevel("level2.txt", false);
								sendBallToStart();
							}
							else if(currentLevel == 2)
							{
								currentLevel++;
								loadLevel("level3.txt", false);
								sendBallToStart();
							}
							else if(currentLevel == 3)
							{
								currentLevel = 1; // change once more levels are created
								loadLevel("level1.txt", false);
								complete = true;
								sendBallToStart();
							}

							if(!complete)
							{
								SoundEngine->play2D("./smallCrowd.wav", false);
							}
							else
							{
								SoundEngine->play2D("./crowedApplause.wav", false);

								game tmp;

								tmp.winner = player1Name;

								bool playerExist = false;
								for(int i = 0; i < topScores.size(); i++)
								{
									if(player1Name == topScores[i].winner)
									{
										playerExist = true;

										if((minutes <= topScores[i].minutes))
										{
											if(seconds <= topScores[i].seconds)
											{
												if(mseconds<= topScores[i].mseconds)
												{
													topScores[i].level = currentLevel;
													topScores[i].hours = hours;
													topScores[i].minutes = minutes;
													topScores[i].seconds = seconds;
													topScores[i].mseconds = mseconds;
												}
											}
										}
									}
								}

								if(!playerExist)
								{
									tmp.hours = hours;
									tmp.minutes = minutes;
									tmp.seconds = seconds;
									tmp.mseconds = mseconds;
									topScores.push_back(tmp);
								}

								hours = minutes = seconds = mseconds = 0;
							}
						}
					}
				}
			}

			// If it is at a hole
			if(board[j][k].type == 'h')
			{
				// Check to see position of sphere 1 and 2,
				btVector3 spherePos = sphereRb->getCenterOfMassPosition();
				btVector3 sphere2Pos = btVector3(0,0,0);

				if(numSpheres == 2)
				{
					sphere2Pos = sphere2Rb->getCenterOfMassPosition();
				}
			
				// If sphere 2 is in the hole, reset both spheres at start			
				if(sphere2Pos.y() <-1)
				{
					sendBallToStart();
				}

				// Otherwise, just do the first
				if(spherePos.y() < -1)
				{
					sendBallToStart();
				}
			}
		}
	}

	// update the state of the scene
	glutPostRedisplay();// call the display callback
}

// Reshapes window size based on w and h
void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;

    // change the viewport to be correct
    glViewport( 0, 0, w, h);

    // update the projection matrix as well
    // see the init function for an explaination
    projection = glm::perspective(45.6f, float(w)/float(h), 0.01f, 100.0f);
}

// keyboard inputs used by keyOps
void keyboard(unsigned char key, int x_pos, int y_pos)
{
	keyStates[key] = true;
}

// Places them at false upon releasing
void keyReleased(unsigned char key, int x_pos, int y_pos)
{
	keyStates[key] = false;
}

void keySpecialReleased(int key, int x_pos, int y_pos)
{
	if(keySpecialStates[GLUT_KEY_UP])
	{
		dynamicsWorld->setGravity(btVector3(0.0, -9.81, 0.0));
	}

	if(keySpecialStates[GLUT_KEY_LEFT])
	{
		dynamicsWorld->setGravity(btVector3(0.0, -9.81, 0.0));
	}

	if(keySpecialStates[GLUT_KEY_RIGHT])
	{
		dynamicsWorld->setGravity(btVector3(0.0, -9.81, 0.0));
	}

	if(keySpecialStates[GLUT_KEY_DOWN])
	{
		dynamicsWorld->setGravity(btVector3(0.0, -9.81, 0.0));
	}

	keySpecialStates[key] = false;
}

// handles keyboard input
void keyOps()
{
	// Wake up the sphere, causes issues at first in which the ball stays still
	sphereRb->activate();

    // ends the program
    if(keyStates[27]) // escape key
    {
    	writeToTopScores();
    	cleanUp();
        exit(0);
    }
	
	// toggles between day and night mode
	if(keyStates['1'])
	{
		ambOn = !ambOn;
		spotOn = !spotOn;
		pointOn = !pointOn;

		keyStates['1'] = false;
	}

	// toggles between normal walls and moving walls
	if(keyStates['2'])
	{
		// Changes difficulty to do things differently
		difficult = !difficult;

		// Reset
		keyStates['2'] = false;
	}

	// toggles a second ball
	if(keyStates['3'])
	{
		// Check to see if the number of balls is 1
		if(numSpheres == 1)
		{
			// if so, increment the number of balls
			numSpheres++;

			// Instantiate the second ball next to the first ball
			{
			// Create collision for the second sphere object
			btCollisionShape* sphereShape = new btSphereShape(0.375);
			btDefaultMotionState* sphereMotionState = new btDefaultMotionState(btTransform(	btQuaternion(0, 0, 0, 1), 
																							btVector3(	sphereRb->getCenterOfMassPosition().x() - 0.75,
																										sphereRb->getCenterOfMassPosition().y(), 
																										sphereRb->getCenterOfMassPosition().z())));

			btScalar mass = 1;
			btVector3 fallInertia(0, 0, 0);
			sphereShape->calculateLocalInertia(mass, fallInertia);
			btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI( mass, 
																		sphereMotionState, 
																		sphereShape, 
																		fallInertia); 
		 	sphere2Rb = new btRigidBody(sphereRigidBodyCI);
		 	sphere2Rb->setRestitution(1.0);
			sphere2Rb->setFriction(0.9);
		  	dynamicsWorld->addRigidBody(sphere2Rb, COL_SPH, sphereCollision);
		  	}	
		}

		// Otherwise remove the second ball from the game
		else if(numSpheres == 2)
		{
			numSpheres--;

			dynamicsWorld->removeRigidBody(sphere2Rb);
		}

		keyStates['3'] = false;
	}
/*
	// toggles ambient light
	if(keyStates['1'])
	{
		ambOn = !ambOn;
		keyStates['1'] = false;
	}

	// toggles directional light
	if(keyStates['2'])
	{
		disOn = !disOn;
		keyStates['2'] = false;
	}
	
	// toggles spotlight
	if(keyStates['3'])
	{
		spotOn = !spotOn;
		keyStates['3'] = false;
	}	

	// toggles point light
	if(keyStates['4'])
	{
		pointOn = !pointOn;
		keyStates['4'] = false;
	}	
*/
/*
	// Add velocity in one direction based on wasd
	if(keyStates['w'])
	{

		if(keyStates['g'])
		{
			addVelocity(-8.0, 0.0, 0.0);
		}
		else
		{
			addVelocity(-4.0, 0.0, 0.0);
		}		
	}

	if(keyStates['a'])
	{
		if(keyStates['g'])
		{
			addVelocity(0.0, 0.0, 8.0);
		}
		else
		{
			addVelocity(0.0, 0.0, 4.0);
		}	
	}

	if(keyStates['s'])
	{
		if(keyStates['g'])
		{
			addVelocity(8.0, 0.0, 0.0);
		}
		else
		{
			addVelocity(4.0, 0.0, 0.0);
		}	
	}

	if(keyStates['d'])
	{
		if(keyStates['g'])
		{
			addVelocity(0.0, 0.0, -8.0);
		}
		else
		{
			addVelocity(0.0, 0.0, -4.0);
		}
	}
		// moves the sphere up. for testing purposes. will remove in demo.
	if(keyStates['e'])
	{
		addVelocity(0.0, 2.0, 0.0);
	}
		*/

	// resets level
	if(keyStates['r'])
	{
		// Puts both balls to the start if existent
		sendBallToStart();

		// Reset the minutes, hours, and seconds
		hours = mseconds = seconds= minutes = 0;
	}

	// skips to the next level
	if(keyStates['n'])
	{
		// State that its moving to the next levle
		transitioning = true;
		
		// Check current level and move accordingly
		if(currentLevel == 1)
		{
			currentLevel++;
			loadLevel("level2.txt", false);
			sendBallToStart();
		}
		else if(currentLevel == 2)
		{
			currentLevel++;
			loadLevel("level3.txt", false);
			sendBallToStart();
		}
		else if(currentLevel == 3)
		{
			currentLevel = 1; // change once more levels are created
			loadLevel("level1.txt", false);
			sendBallToStart();
		}

		keyStates['n'] = false;
	}

	if(keyStates['i'])
	{
		//cameraPos[1] -= 0.8;
		//keyStates['i'] = false;
	}

	if(keyStates['k'])
	{
		//cameraPos[1] += 0.8;
		//keyStates['k'] = false;
	}

	if(keyStates['u'])
	{
		//cameraPos[0] += 0.8;
		//keyStates['u'] = false;
	}

	if(keyStates['j'])
	{
		//cameraPos[0] -= 0.8;
		//keyStates['j'] = false;
	}

	if(keyStates['o'])
	{
		//cameraPos[2] += 0.8;
		//keyStates['o'] = false;
	}

	if(keyStates['l'])
	{
		//cameraPos[2] -= 0.8;
		//keyStates['l'] = false;
	}	

	// gets out of title screen and into game screen or into something else
	if(keyStates['q'])
	{
		titleScreen = 0;
		pauseGame = 0 ;
	}

	// Change the theme to golden age and play nice music
	if(keyStates['4'])
	{
		// Stop the sound and play the song
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("./Study_Music_Project_-_Midnight_Coffee_music.wav", true);

		stylishWood = false;
		goldenAge = true;
	}

	// Change the theme to stylish wood amd play the same old breakout song
	if(keyStates['5'])
	{
		// Stop the song and play
		SoundEngine->stopAllSounds();
		SoundEngine->play2D("./breakout.wav", true);

		stylishWood = true;
		goldenAge = false;
	}

	// Check for key states to go back to title screen
	if(keyStates['.'])
	{
		if(showTopScore == 1 && titleScreen == 0)
		{
		titleScreen = 1;
		showTopScore = 0;
		}
	}

	//This is how to start the game
	else if(keyStates['9'])
	{
		if(titleScreen == 1 &&  showTopScore == 0)
		{
			showTopScore = 1;
			titleScreen = 0;
		}
	}
}

bool initialize()
{
	// Variables
	shader frag_shade;
	shader vert_shade;

	// Instantiate camera position and things to focus
	cameraPos[0] = 5.0;
	cameraPos[1] = 20.0;
	cameraPos[2] = -5.0;
	focusPos[0] = model[0][0][0];
	focusPos[1] = model[0][0][1];
	focusPos[2] = model[0][0][2];
	cameraAngle[0] = 0.0;
	cameraAngle[1] = 1.0;
	cameraAngle[2] = 0.0;

	// Go through and prim the values
	for(int i = 0; i < numModels; i++)
	{
		numVert[i] = 0;
	}

	for(int i = 0; i < 256; i++)
	{
		keyStates[i] = false;
	}

	// create board
	loadLevel("level1.txt", true);

	// This starts loading the different objects 
	Vertex *sphereObj = objLoader("sphere.obj", numVert[0] , sphereMesh);
	Vertex *sphere2Obj = objLoader("sphere.obj", numVert[2] , sphere2Mesh);
	Vertex *cubeObj = objLoader("cube.obj", cubeVert, cubeMesh);
	Vertex *holeObj = objLoader("cubeWithHole.obj", holeVert, holeMesh);
	Vertex *frameObj = objLoader("frame.obj", numVert[1], frameMesh);
	Vertex *boxObj = objLoader("box.obj", numVert[3], boxMesh);

	// Generate buffers for all of the objects 
	glGenBuffers(2 + numModels, &vbo_geometry[0]);

	// Start binding those buffers to those locations

	// Bind the Cube
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * cubeVert, cubeObj, GL_STATIC_DRAW);

	// Bind the Sphere
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVert[0], sphereObj, GL_STATIC_DRAW);

	// Bind the Frame
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVert[1], frameObj, GL_STATIC_DRAW);

	// Bind the Cube with the hole
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * holeVert, holeObj, GL_STATIC_DRAW);

	// Bind the second Sphere
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVert[2], sphere2Obj, GL_STATIC_DRAW);

	// Bind the box
	glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry[5]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVert[3], boxObj, GL_STATIC_DRAW);

	// Allocate memory for the magick pointers so they can get the textures
	texture = new GLuint[(numModels + 4)*3];
	bigBlob = new Magick :: Blob[(numModels + 4)*3];	

	// Generate spaces for those texture

	glGenTextures((numModels + 4)*3, texture);

	// Create a image object and then have it read it, create textures bind it, and then do other gibberish things
	// Stylish wood texture

	loadImage("color3.jpg", 0);
	loadImage("grey.jpg", 1);
	loadImage("frame.jpg", 2);
	loadImage("color3.jpg", 3);
	loadImage("green.jpg", 4);
	loadImage("red.jpg", 5);
	loadImage("metal.jpg", 6);

	// Golden Age
	loadImage("maroon.jpg", 7);
	loadImage("golden.jpg", 8);
	loadImage("blue.jpg", 9);

	// Intialize bullet
	// Tells bullet how to check for collisions between bojects
	broadphase = new btDbvtBroadphase(); 
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new  btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,-9.81 ,0));

	{
	// Create collision for the ground plane
	btCollisionShape *planeShape = new btStaticPlaneShape ( btVector3(0, 1, 0 ), -1 );
	btDefaultMotionState *planeMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,-1,0)));
	btScalar mass(0);
	btVector3 inertia(0,0,0);
	planeShape -> calculateLocalInertia(mass, inertia);
	btRigidBody::btRigidBodyConstructionInfo
                planeRigidBodyCI(0, planeMotionState, planeShape, inertia);;
    planeRigidBody = new btRigidBody(planeRigidBodyCI);
	dynamicsWorld->addRigidBody(planeRigidBody, COL_PLANE, planeCollision );
	}

	// Loop through and instantiate the cube for the board at certain heights and widths
	for(int j = 0; j < cubesH; j++)
	{
		for(int k = 0; k < cubesW; k++)
		{
			if(board[j][k].type == 's' || board[j][k].type == 'b' || board[j][k].type == 'e')	
			{
				btCollisionShape *shape = new btBvhTriangleMeshShape(cubeMesh, true);
				btDefaultMotionState *shapeMotionState = NULL;
				shapeMotionState = new btDefaultMotionState( btTransform( 
									                       btQuaternion( 0, 0, 0, 1 ),
									                       btVector3(j * 2, board[j][k].height, k * 2) ) );
				btScalar mass(0);
				btRigidBody::btRigidBodyConstructionInfo
				shapeRigidBodyCI( 0, shapeMotionState, shape, btVector3( 0, 0, 0 ) ); 
				cubeRb[j * cubesW + k] = new btRigidBody( shapeRigidBodyCI );
				cubeRb[j * cubesW + k]->setRestitution( 0.0 );
				cubeRb[j * cubesW + k]->setFriction(0.9);
				dynamicsWorld->addRigidBody( 	cubeRb[j * cubesW + k],
								       			COL_CUBE, 
									       		cubeCollision );  
			}
			else if(board[j][k].type == 'h')
			{
				btCollisionShape *shape = new btBvhTriangleMeshShape(holeMesh, true);
				btDefaultMotionState *shapeMotionState = NULL;
				shapeMotionState = new btDefaultMotionState( btTransform( 
									                       btQuaternion( 0, 0, 0, 1 ),
									                       btVector3(j * 2, board[j][k].height, k * 2) ) );
				btScalar mass(0);
				btRigidBody::btRigidBodyConstructionInfo
				shapeRigidBodyCI( 0, shapeMotionState, shape, btVector3( 0, 0, 0 ) ); 
				cubeRb[j * cubesW + k] = new btRigidBody( shapeRigidBodyCI );
				cubeRb[j * cubesW + k]->setRestitution( 0.0 );
				cubeRb[j * cubesW + k]->setFriction(0.9);
				dynamicsWorld->addRigidBody( 	cubeRb[j * cubesW + k],
								       			COL_CUBE, 
									       		cubeCollision );  
			}			
		}
	}

	{
	// Create collision for the sphere object
	btCollisionShape* sphereShape = new btSphereShape(0.375);
	btDefaultMotionState* sphereMotionState = new btDefaultMotionState(btTransform(	btQuaternion(0, 0, 0, 1), 
																					btVector3(-1,4,-1)));
    btScalar mass = 1;
    btVector3 fallInertia(0, 0, 0);
    sphereShape->calculateLocalInertia(mass, fallInertia);
	btRigidBody::btRigidBodyConstructionInfo sphereRigidBodyCI( mass, 
																sphereMotionState, 
																sphereShape, 
																fallInertia); 
 	sphereRb = new btRigidBody(sphereRigidBodyCI);
 	sphereRb->setRestitution(1.0);
	sphereRb->setFriction(0.9);
  	dynamicsWorld->addRigidBody(sphereRb, COL_SPH, sphereCollision);
  	}

	// set the sphere to start at the starting cube
	sendBallToStart();

   	// This is for the frame
   	{
	btCollisionShape *shape = new btBvhTriangleMeshShape(frameMesh, 
                                                                  true);
	btDefaultMotionState *shapeMotionState = NULL;
	shapeMotionState = new btDefaultMotionState( btTransform( 
		                                       btQuaternion( 0, 0, 0, 1 ),
		                                       btVector3( 0, 0, 0) ) );
	btScalar mass(0);
	btRigidBody::btRigidBodyConstructionInfo
	shapeRigidBodyCI( 0, shapeMotionState, shape, btVector3( 0, 0, 0 ) ); 
	frameRb = new btRigidBody( shapeRigidBodyCI );
	dynamicsWorld->addRigidBody( 	frameRb,
	                       			COL_FRAME, 
		                       		frameCollision );       
	}

	// This is for the bounding box
   	{
	btCollisionShape *shape = new btBvhTriangleMeshShape(boxMesh, 
                                                                  true);
	btDefaultMotionState *shapeMotionState = NULL;
	shapeMotionState = new btDefaultMotionState( btTransform( 
		                                       btQuaternion( 0, 0, 0, 1 ),
		                                       btVector3( 0, 0, 0) ) );
	btScalar mass(0);
	btRigidBody::btRigidBodyConstructionInfo
	shapeRigidBodyCI( 0, shapeMotionState, shape, btVector3( 0, 0, 0 ) ); 
	boxRb = new btRigidBody( shapeRigidBodyCI );
	dynamicsWorld->addRigidBody( 	boxRb,
	                       			COL_FRAME, 
		                       		frameCollision );       
	}

	// Shader stuff starts here
	program = glCreateProgram();

	// Get file name
	vert_shade.setShaderFileName("./bin/shader.vs");
	frag_shade.setShaderFileName("./bin/shader.fs");

	// Read it in. if it fails, check other location
	if(!vert_shade.readIn())
	{
		vert_shade.setShaderFileName("shader.vs");
		vert_shade.readIn();
	}

	if(!frag_shade.readIn())
	{
		frag_shade.setShaderFileName("shader.fs");
		frag_shade.readIn();
	}

	// assign it's position
	GLuint vertex_shader = vert_shade.shaderLoader( GL_VERTEX_SHADER );
	GLuint fragment_shader = frag_shade.shaderLoader( GL_FRAGMENT_SHADER) ;
	GLint shader_status; 

	// attach the programs to the shaders
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        return false;
    }

	// then start getting the attribute variables from the vertex shader
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
    }

    loc_uv = glGetAttribLocation(program,
                    const_cast<const char*>("v_texture"));
    if(loc_uv == -1)
    {
        std::cerr << "[F] UV_POSITION NOT FOUND" << std::endl;
        //return false;
    } 
	
    loc_mvpmat = glGetUniformLocation(program, const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        //return false;
    }

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    loc_normal = glGetAttribLocation(program, const_cast<const char*>("v_normals"));
    if(loc_normal == -1)
	{
		std :: cerr << "Bad normals" << endl;
	}

	loc_m = glGetUniformLocation(program, const_cast<const char*>("model"));
    if(loc_m == -1)
    {
        std::cerr << "[F] BAD MODELO" << std::endl;
        //return false;
    }

    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    return true;
}

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(numModels + numCubes, vbo_geometry);

    // Rmemove Magick first
	delete []bigBlob;
	delete []texture;

	// Then go to the bullet stuff
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}

//returns the time delta
float getDT()
{
	float ret = 0;

	t2 = std::chrono::high_resolution_clock::now();
	ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
	t1 = std::chrono::high_resolution_clock::now();

	return ret;
}

void menu(int id)
{
	switch(id)
	{
		case 1:
			if(titleScreen == false)
			{
				pauseGame = false;
			}
	
			break;

		case 2:
			pauseGame = true;
			break;

		case 3:
			writeToTopScores();
			exit(0);
			break;
	}
}

Vertex *objLoader(string imageFile, int &numberOfVertices, btTriangleMesh *& sendBackMesh)
{
	/*
		aiMesh** mMeshes - contains a 2-dimensional array of object meshes
		aiTexture** mTextures - contains a 2-dimensional array of textures
		int mNumMeshes and mNumTextures - contain the number of the respective objects in the scene
	*/

	// Intialize Assimp
	Assimp :: Importer importer;

	// Triangles are my favorite
	const aiScene *myScene = importer.ReadFile(imageFile, aiProcess_Triangulate);

	// Create btVector3
	btVector3 triArray[3];

	// Allocate memory to sendBackMesh
	sendBackMesh = new btTriangleMesh();

	if(!myScene)
	{
		// if default location doesn't work, try adding bin in front
		string otherLoc = "./bin/" ;
		otherLoc.append(imageFile);

		myScene = importer.ReadFile(otherLoc, aiProcess_Triangulate);

		if(!myScene) // at this point both locations didn't work
		{
			cout << importer.GetErrorString() << endl;
			exit(1);
		}
	}

	// Check to see if there are any meshes in the scene; if there is, grab the first one
	if(myScene->mNumMeshes > 0)
	{
		aiMesh *mesh = myScene->mMeshes[0];// This grabs the mesh from the first scene
		
		// Check the number of vertices, each face is a triangle which has 3 vertices
		numberOfVertices = mesh->mNumFaces * 3;

		// Create a array
		Vertex *outVertex = new Vertex[numberOfVertices];

		int out= 0;
		for(unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			// Get face reference in order to extract it's value
			const aiFace& face = mesh->mFaces[i];			
			for(int j = 0; j < 3; j++)
			{			
				if(mesh->HasPositions())
				{
					// First get the vertices from each of the faces
					aiVector3D position = mesh->mVertices[face.mIndices[j]];

					outVertex[out].position[0] = position.x;
					outVertex[out].position[1] = position.y;
					outVertex[out].position[2] = position.z;
			        triArray[j] = btVector3(position.x, position.y, position.z); 
				}

				if(mesh->HasNormals())
				{
					// Then get the normals from each of the faces
					aiVector3D normal = mesh->mNormals[face.mIndices[j]];
		
					outVertex[out].normals[0] = normal.x;
					outVertex[out].normals[0] = normal.y;
					outVertex[out].normals[0] = normal.z;
				}

				if(mesh->HasTextureCoords(0))
				{
					aiVector3D uv = mesh->mTextureCoords[0][face.mIndices[j]];
					outVertex[out].uv[0] = uv.x;
					outVertex[out].uv[1] = uv.y;
				}

				// One day I will use the textures, but not today.
				out++;
			}
			sendBackMesh->addTriangle(triArray[0], triArray[1], triArray[2]); 
		}
			return outVertex;
	}	
}

void processSpecialKeys(int key, int xx, int yy)
{
	keySpecialStates[key] = true;
}

void specialKeyOps()
{
	if(keySpecialStates[GLUT_KEY_UP])
	{
		sphereRb->activate();
		if(numSpheres == 2)
		{
			sphere2Rb->activate();
		}
		dynamicsWorld->setGravity(btVector3(-4.905,-9.81,0.0));
	}

	if(keySpecialStates[GLUT_KEY_LEFT])
	{
		sphereRb->activate();
				if(numSpheres == 2)
		{
			sphere2Rb->activate();
		}
		dynamicsWorld->setGravity(btVector3(0.0,-9.81,4.905));
	}

	if(keySpecialStates[GLUT_KEY_RIGHT])
	{
		sphereRb->activate();
		if(numSpheres == 2)
		{
			sphere2Rb->activate();
		}
		dynamicsWorld->setGravity(btVector3(0.0,-9.81,-4.905));
	}

	if(keySpecialStates[GLUT_KEY_DOWN])
	{
		sphereRb->activate();
		if(numSpheres == 2)
		{
			sphere2Rb->activate();
		}
		dynamicsWorld->setGravity(btVector3(4.905,-9.81,0.0));
	}
}

void updateRb(btRigidBody* body, int position)
{
    // This is for bullet
    btTransform trans;
    btScalar m[16];

    // Get the current position in the real world
	body->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(m);

    // Make a model matrix for the specific object
    model[position] = glm::make_mat4(m);

    // Have it focus on the position of the sphere
	if(modelToFocus == position)
	{
		cameraPos[0] = 8.0 + model[modelToFocus][3][0];
		cameraPos[1] = 8.0 + model[modelToFocus][3][1];
		cameraPos[2] = 0.0 + model[modelToFocus][3][2];

		focusPos[0] = model[modelToFocus][3][0];
		focusPos[1] = model[modelToFocus][3][1] + cameraAngle[0];
		focusPos[2] = model[modelToFocus][3][2];
	}
}

void loadImage(string file, int t)
{
	// Create a magick image
	Magick::Image image;

	// Check to see if it fails
	try
	{
		image.read(file);
		image.write(&bigBlob[t], "RGBA");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[t]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.columns(), image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bigBlob[t].data());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	// If happens then do something else
	catch(Magick::ErrorBlob &error)
	{
		string otherLoc = "./bin/";
		otherLoc.append(file);
		image.read(otherLoc);
		image.write(&bigBlob[t], "RGBA");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[t]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.columns(), image.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bigBlob[t].data());
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

// Give the sphere velocity at direction z
void addVelocity(float x, float y, float z)
{
	// Wake up the sphere if asleep
	sphereRb->activate();

	// Set the linear velocity
	sphereRb->setLinearVelocity(btVector3(x,
										y,
										z));
}

// Load the level 
void loadLevel(string filename, bool immediate)
{
	// Open file stream
	ifstream fin(filename.c_str());

	// Say the level the game is loading
	cout << "Loading level: " << filename << endl;

	// Check to see if file is good
	if(fin.good())
	{
		// Loop through twice and read in the cube data
		for(int i  = 0; i < cubesW; i++)	
		{
			for(int j  = 0; j < cubesH; j++)	
			{
				if(immediate)
				{
					fin >> board[i][cubesH - j - 1].type >> board[i][cubesH - j - 1].height;
				}
				else
				{
					fin >> board[i][cubesH - j - 1].type >> board[i][cubesH - j - 1].targetHeight;
				}
			}
		}
	}
}

// Change the angle of the camera
void updateCameraAngle()
{
	float increment = 0.1;

	// Get the gravity of the dynamic world
	btVector3 gravity = dynamicsWorld->getGravity();
	
	// Set the individual gravity to equal
	gravity[0] *= -1;
	gravity[1] *= -1;
	gravity[2] *= -1;

	// gives pitch effect
	if(gravity[0] > cameraAngle[0] + increment)
	{
		cameraAngle[0] += increment;
	}
	else if(gravity[0] < cameraAngle[0] - increment)
	{
		cameraAngle[0] -= increment;
	}

	// limits pitch effect
	if(cameraAngle[0] > 2.0)
	{
		cameraAngle[0] = 2.0;
	}

	// yaw?
	if(gravity[1] > cameraAngle[1] + increment)
	{
		cameraAngle[1] += increment;
	}
	else if(gravity[1] < cameraAngle[1] - increment)
	{
		cameraAngle[1] -= increment;
	}

	// gives roll effect
	if(gravity[2] > cameraAngle[2] + increment)
	{
		cameraAngle[2] += increment;
	}
	else if(gravity[2] < cameraAngle[2] - increment)
	{
		cameraAngle[2] -= increment;
	}

	// limits roll effect
	if(cameraAngle[2] > 2.0)
	{
		cameraAngle[2] = 2.0;
	}
}

// Resets the sphere to at the starting point
bool sendBallToStart()
{
	for(int i = 0; i < cubesW; i++)
	{
		for(int j = 0; j < cubesH; j++)
		{
			// note: if for some reason there are 2 starting points in the level file,
			// the first one it encounters will be the one the sphere gets sent to
			if(board[i][j].type == 'b')
			{
				{
					btDefaultMotionState *sphereMotionState = NULL;		
					// position the sphere on the starting point
		  			sphereMotionState = new btDefaultMotionState(btTransform(
		  														btQuaternion(0,0,0,1),
		  														btVector3(	i * 2, 
																			board[cubesW][cubesH].height + 3.0,
																			j * 2)));
					sphereRb->setLinearVelocity(btVector3(0,-1,0)); // stops most of the movement
					sphereRb->setMotionState(sphereMotionState);
				}
				
				if(numSpheres == 2)
				{
					btDefaultMotionState *sphereMotionState = NULL;		
					// position the sphere on the starting point
		  			sphereMotionState = new btDefaultMotionState(btTransform(
		  														btQuaternion(0,0,0,1),
		  														btVector3(	(i * 2), 
																			board[cubesW][cubesH].height + 4.0,
																			(j * 2))));
					sphere2Rb->setLinearVelocity(btVector3(0,-1,0)); // stops most of the movement
					sphere2Rb->setMotionState(sphereMotionState);					
				}
				return true;
			}
		}
	}

	return false;	
}


// Read in top Scores from the file
bool readInTopScores()
{
	// File stream variable, open a variable called topSccores
	ifstream fin("topScores.txt");

	// If the file exist then read through it for the first 10 games
	if(fin.good())
	{
		while(!fin.eof())
		{
			game tmp;
			char junk;

			fin >> tmp.winner >>  tmp.level >> tmp.hours >> tmp.minutes >> tmp.seconds >> tmp.mseconds ;

			if(fin.peek())
			{

			}

			// Check to see if the string is null
			if(!tmp.winner.empty())
			{
				topScores.push_back(tmp);
			}
		}
	}
	// Close the filestream
	fin.close();
}

// Write the max scores to file
void writeToTopScores()
{
	// Open a file out stream with the name topScores
	ofstream fout("topScores.txt");

	// Check to see if opened successfully
	if(fout.good())
	{
		// Loop through write the data
		for(int i = 0; i < topScores.size(); i++)
		{
			fout << topScores[i].winner << " " << topScores[i].level << " " << topScores[i].hours << " " << topScores[i].minutes << " " << topScores[i].seconds << " " << topScores[i].mseconds << endl;
		}	
	}
	else// Otherwise quit
	{
		cout << "Cannot open .txt file for some reason" << endl;
	}

	// Close fout stream
	fout.close();
}

// The thing that moves everything to the right place when changing levels
void transition()
{
	// Check to see if transitioning is true/ only occurs in update function
	if(transitioning)
	{
		// If so set it to false, so it does not occur multiple times
		transitioning = false;

		// Loop through and set things accordingly
		for(int i = 0; i < cubesW; i++)
		{
			for(int j = 0; j < cubesH; j++)
			{
				if(board[i][j].targetHeight != -1)
				{
					if(board[i][j].height >= board[i][j].targetHeight - 0.05 && board[i][j].height <= board[i][j].targetHeight + 0.05)
					{
						board[i][j].height = board[i][j].targetHeight;
						board[i][j].targetHeight = -1;
					}
					else if(board[i][j].height < board[i][j].targetHeight)
					{
						transitioning = true;
						board[i][j].height += 0.1;
					}
					else if(board[i][j].height > board[i][j].targetHeight)
					{
						transitioning = true;
						board[i][j].height -= 0.1;
					}

					// removes old rigid body. needed for switching bewteen cube with hole and cube without hole
					dynamicsWorld->removeRigidBody(cubeRb[i * cubesW + j]);

					if(board[i][j].type == 's' || board[i][j].type == 'b' || board[i][j].type == 'e')	
					{
						btCollisionShape *shape = new btBvhTriangleMeshShape(cubeMesh, true);
						btDefaultMotionState *shapeMotionState = NULL;
						shapeMotionState = new btDefaultMotionState( btTransform( 
													               btQuaternion( 0, 0, 0, 1 ),
													               btVector3(i * 2, board[i][j].height, j * 2) ) );
						btScalar mass(0);
						btRigidBody::btRigidBodyConstructionInfo
						shapeRigidBodyCI( 0, shapeMotionState, shape, btVector3( 0, 0, 0 ) ); 
						cubeRb[i * cubesW + j] = new btRigidBody( shapeRigidBodyCI );
						cubeRb[i * cubesW + j]->setRestitution( 0.0 );
						cubeRb[i * cubesW + j]->setFriction(0.9);
						dynamicsWorld->addRigidBody( 	cubeRb[i * cubesW + j],
											   			COL_CUBE, 
												   		cubeCollision );  
					}
					else if(board[i][j].type == 'h')
					{
						btCollisionShape *shape = new btBvhTriangleMeshShape(holeMesh, true);
						btDefaultMotionState *shapeMotionState = NULL;
						shapeMotionState = new btDefaultMotionState( btTransform( 
													               btQuaternion( 0, 0, 0, 1 ),
													               btVector3(i * 2, board[i][j].height, j * 2) ) );
						btScalar mass(0);
						btRigidBody::btRigidBodyConstructionInfo
						shapeRigidBodyCI( 0, shapeMotionState, shape, btVector3( 0, 0, 0 ) ); 
						cubeRb[i * cubesW + j] = new btRigidBody( shapeRigidBodyCI );
						cubeRb[i * cubesW + j]->setRestitution( 0.0 );
						cubeRb[i * cubesW + j]->setFriction(0.9);
						dynamicsWorld->addRigidBody( 	cubeRb[i * cubesW + j],
											   			COL_CUBE, 
												   		cubeCollision );
					}
				}
			}
		}
	}
}

// Function for diffciulty setting, in which the height of the cube is based on the sphere's promixity
float getDistToSphere(int i, int j)
{
	float cubeX = cubeRb[i * cubesH + j]->getCenterOfMassPosition().x();
	float cubeZ = cubeRb[i * cubesH + j]->getCenterOfMassPosition().z();

	float distToSphere = hypot( ( sphereRb->getCenterOfMassPosition().x() - cubeX), 
								( sphereRb->getCenterOfMassPosition().z() - cubeZ));

	return distToSphere;
}

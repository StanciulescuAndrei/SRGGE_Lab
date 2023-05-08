#include <GL/glew.h>
#include <GL/glut.h>
#include "Application.h"
#include "Simplifier.h"
#include <stdlib.h>
#include "TileMap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Remove console (only works in Visual Studio)
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")


#define TIME_PER_FRAME 1000.f / 60.f // Approx. 60 fps


static int prevTime;
static Application app; // This object represents our whole app
static Simplifier simplifier;


// If a key is pressed this callback is called

static void keyboardDownCallback(unsigned char key, int x, int y)
{
	Application::instance().keyPressed(key);
}

// If a key is released this callback is called

static void keyboardUpCallback(unsigned char key, int x, int y)
{
	Application::instance().keyReleased(key);
}

// If a special key is pressed this callback is called

static void specialDownCallback(int key, int x, int y)
{
	Application::instance().specialKeyPressed(key);
}

// If a special key is released this callback is called

static void specialUpCallback(int key, int x, int y)
{
	Application::instance().specialKeyReleased(key);
}

// Same for changes in mouse cursor position

static void motionCallback(int x, int y)
{
	Application::instance().mouseMove(x, y);
}

// Same for mouse button presses or releases

static void mouseCallback(int button, int state, int x, int y)
{
  int buttonId;
  
  switch(button)
  {
  case GLUT_LEFT_BUTTON:
    buttonId = 0;
    break;
  case GLUT_RIGHT_BUTTON:
    buttonId = 1;
    break;
  case GLUT_MIDDLE_BUTTON:
    buttonId = 2;
    break;
  }

	if(state == GLUT_DOWN)
		Application::instance().mousePress(buttonId);
	else if(state == GLUT_UP)
		Application::instance().mouseRelease(buttonId);
}

// Resizing the window calls this function

static void resizeCallback(int width, int height)
{
  Application::instance().resize(width, height);
}

// Called whenever the windows needs to be redrawn
// This includes trying to render the scene at 60 FPS

static void drawCallback()
{
	Application::instance().render();
	glutSwapBuffers();
}

// When there is no other event to be processed GLUT calls this function
// Here we update the scene and order GLUT to redraw the window every 
// 60th of a second, so that we get 60 FPS

static void idleCallback()
{
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = currentTime - prevTime;
	
	if(deltaTime > TIME_PER_FRAME)
	{
		// Every time we enter here is equivalent to a game loop execution
		if(!Application::instance().update(deltaTime))
			exit(0);
		prevTime = currentTime;
		glutPostRedisplay();
	}
}


int main(int argc, char **argv)
{
	// GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(640, 480);
	glutCreateWindow(argv[0]);

	// Register all callbacks. GLUT will call the functions whenever 
	// the corresponding event is triggered
	glutReshapeFunc(resizeCallback);
	glutDisplayFunc(drawCallback);
	glutIdleFunc(idleCallback);
	glutKeyboardFunc(keyboardDownCallback);
	glutKeyboardUpFunc(keyboardUpCallback);
	glutSpecialFunc(specialDownCallback);
	glutSpecialUpFunc(specialUpCallback);
	glutMouseFunc(mouseCallback);
	glutPassiveMotionFunc(motionCallback);

	// GLEW will take care of OpenGL extension functions
	glewExperimental = GL_TRUE;
	glewInit();

	// Load image for tilemap
	int w, h, comp;
	uint8_t* img = stbi_load(  "../../map/tilemap.bmp", &w, &h, &comp, 3);
	printf("Tilemap height: %d\n", h);
	printf("Tilemap height: %d\n", w);
	printf("Tilemap comp: %d\n", comp);
	TileMap map(img, w, h, comp);
	
	// Application instance initialization
	Application::instance().init(map);
	if(argc == 1){
		Application::instance().loadMesh("../../models/moai_LOD6.ply");
		Application::instance().loadMesh("../../models/dragon_LOD6.ply");
		Application::instance().loadMesh("../../models/frog_LOD6.ply");
		Application::instance().loadMesh("../../models/Armadillo_LOD6.ply");
		Application::instance().loadMesh("../../models/lucy_LOD6.ply");
	}
	else if(argc == 2){
	  Application::instance().loadMesh(argv[1]);
	}
	else if(argc == 3 && strcmp(argv[1], "simplify") == 0){
		printf("Starting LOD generation...\n");
		Simplifier::instance().loadMesh(argv[2]);
		printf("Computing LOD...\n");
		Simplifier::instance().computeLODs(3);
		printf("Done...\n");
		return 0;
	}
	else{
		printf("Incorrect usage!\n");
		return 0;
	}

	prevTime = glutGet(GLUT_ELAPSED_TIME);
	// GLUT gains control of the application
	glutMainLoop();

	return 0;
}




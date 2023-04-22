#ifndef _APPLICATION_INCLUDE
#define _APPLICATION_INCLUDE


#include "Scene.h"
#include "TileMap.h"


// Application is a singleton (a class with a single instance) that represents our whole app


class Application
{

public:
	Application() {}
	
	
	static Application &instance()
	{
		static Application G;
	
		return G;
	}
	
	void init(TileMap tilemap);
	bool loadMesh(const char *filename);
	bool update(int deltaTime);
	void render();
	
	void resize(int width, int height);
	
	// Input callback methods
	void keyPressed(int key);
	void keyReleased(int key);
	void specialKeyPressed(int key);
	void specialKeyReleased(int key);
	void mouseMove(int x, int y);
	void mousePress(int button);
	void mouseRelease(int button);
	
	bool getKey(int key) const;
	bool getSpecialKey(int key) const;

private:
	bool bPlay;                       // Continue?
	Scene scene;                      // Scene to render
	bool keys[256], specialKeys[256]; // Store key states so that 
	                                  // we can have access at any time

	glm::ivec2 previousMousePos; 			// Previous mouse position
	bool mouseButtons[3];             // State of mouse buttons

	bool bNavigation;                 // Navigation enabled?
	bool bFullscreen; 								// Fullscreen enabled?
	glm::ivec2 windowSize; 						// Window size before going fullscreen
	int32_t start_time = 0;
	int32_t end_time = 0;
	int16_t framecounter = 0; 
	int16_t num_instances = 1;                                 
};


#endif // _APPLICATION_INCLUDE



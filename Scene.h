#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "VectorCamera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init();
	bool loadMesh(const char *filename);
	void update(int deltaTime);
	void render();

  VectorCamera &getCamera();

private:
	void initShaders();
	void computeModelViewMatrix();
	
	void renderRoom();

private:
  VectorCamera camera;
	TriangleMesh *cube, *mesh;
	ShaderProgram basicProgram;
	float currentTime;

};


#endif // _SCENE_INCLUDE


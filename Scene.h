#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "VectorCamera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "TileMap.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init(TileMap map);
	bool loadMesh(const char *filename);
	void loadMap(TileMap _map);
	void update(int deltaTime);
	void render(uint8_t num_instances);

  VectorCamera &getCamera();

private:
	void initShaders();
	void computeModelViewMatrix();
	
	void renderRoom();

private:
  VectorCamera camera;
	TriangleMesh *cube, *mesh;
	ShaderProgram basicProgram;
	TileMap tilemap;
	float currentTime;

};


#endif // _SCENE_INCLUDE


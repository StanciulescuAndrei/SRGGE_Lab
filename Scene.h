#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "VectorCamera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "TileMap.h"
#include "RenderableEntity.h"
#include <vector>


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init(TileMap map);
	bool loadMesh(const char *filename, uint8_t id);
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
	TriangleMesh *cube;
	std::vector<RenderableEntity *> objects;
	ShaderProgram basicProgram;
	TileMap tilemap;
	std::vector<std::vector<uint32_t>> cellVisibility;
	float currentTime;
	uint8_t object_codes[5] = {38, 59, 82, 106, 132};
};


#endif // _SCENE_INCLUDE


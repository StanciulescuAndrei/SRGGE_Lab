#include <iostream>
#include <fstream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <tuple>
#include <algorithm>
#include "Scene.h"
#include "PLYReader.h"

Scene::Scene()
{
	cube = NULL;
}

Scene::~Scene()
{
	if (cube != NULL)
		delete cube;
	for (RenderableEntity *re : objects)
	{
		delete re;
	}
}

// Initialize the scene. This includes the cube we will use to render
// the floor and walls, as well as the camera.

void Scene::init(TileMap _tilemap)
{
	initShaders();
	cube = new TriangleMesh();
	cube->buildCube();
	cube->sendToOpenGL(basicProgram);
	currentTime = 0.0f;
	tilemap = _tilemap;

	camera.init(glm::vec3(0.f, 0.5f, 2.f));

	// Load cell visibility
	std::ifstream in("../../map/visibility.txt");
	for (int i = 0; i < tilemap.height * tilemap.width; i++)
	{
		std::vector<uint32_t> tmpBuffer;
		for (uint32_t pos = 0; pos < tilemap.height * tilemap.width; pos++)
		{
			uint32_t tmp;
			in >> tmp;
			if (tmp)
				tmpBuffer.push_back(pos);
		}
		cellVisibility.push_back(tmpBuffer);
	}
}

// Loads the mesh into CPU memory and sends it to GPU memory (using GL)

bool Scene::loadMesh(const char *filename, uint8_t id)
{
	RenderableEntity *re = new RenderableEntity(filename, id, basicProgram);
	objects.push_back(re);
	return true;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
}

// Render the scene. First the room, then the mesh if there is one loaded.

void Scene::render(uint8_t num_instances)
{
	const uint32_t triangleBudget = 6e+6; // Budged of 6 million triangles for each frame rendered

	glm::mat3 normalMatrix;

	basicProgram.use();
	basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());

	renderRoom();

	if (objects.size() > 0)
	{
		glm::mat4 modelView;

		glm::vec3 campos = camera.position;
		float newx = campos.x + 0.5f;
		float newz = campos.z + 0.5f;

		newx = std::max(newx, 0.0f);
		newz = std::max(newz, 0.0f);

		newx = std::min(newx, tilemap.width + 1.0f);
		newz = std::min(newz, tilemap.height + 1.0f);

		camera.setPosition(newx - 0.5f, newz - 0.5f);

		// Entity ID, distance to camera (for sorting), position on grid, LOD
		std::vector<std::tuple<uint8_t, float, glm::ivec2, uint8_t>> renderList;

		uint32_t cameraCellIndex = glm::floor(newz) + glm::floor(newx) * tilemap.width;

		uint32_t crtTriBudget = 0;

		for (auto visibleCell : cellVisibility[cameraCellIndex])
		{
			int x = visibleCell % tilemap.width;
			int y = visibleCell / tilemap.width;
			bool statue = (tilemap.GetTile(x, y) > 0 && tilemap.GetTile(x, y) < 255);
			if (statue)
			{
				for (int obj_id = 0; obj_id < objects.size(); obj_id++)
				{
					if (object_codes[obj_id] == tilemap.GetTile(x, y))
					{
						bool frustumVisible = false;
						// Test for frustum culling using radar-like method
						glm::vec3 cameraDirection(sin(M_PI * camera.angleDirection / 180.f), 0.f, cos(M_PI * camera.angleDirection / 180.f));
						std::vector<glm::vec3> cellCorners = {
												glm::vec3(0.0f, 0.0f, 0.0f),
												glm::vec3(1.0f, 0.0f, 0.0f),
												glm::vec3(0.0f, 0.0f, 1.0f),
												glm::vec3(1.0f, 0.0f, 1.0f)
												};

						for(auto cellCorner : cellCorners){
							glm::vec3 objectVector = glm::normalize(glm::vec3(x, 0, y) + cellCorner - camera.position);

							float angle = glm::acos(glm::dot(cameraDirection, objectVector));
							if(glm::abs(angle) <= M_PI / 3){
								frustumVisible = true;
							}
						}
						

						if(frustumVisible == true){
							std::tuple<uint8_t, float, glm::ivec2, uint8_t> renderCandidate;
							float distance = glm::length(glm::vec2(newx, newz) - glm::vec2(x, y));
							renderCandidate = std::make_tuple(obj_id, distance, glm::ivec2(x, y), 0);

							renderList.push_back(renderCandidate);
							crtTriBudget += objects[obj_id]->getNumTriangles(0);
						}
						
					}
				}
			}
		}

		bool improved = true;
		// Maybe all entities are at max LOD and the budget is not finished... avoid an infinite loop
		while (improved)
		{
			improved = false;

			// Sort by the potential improvement to visual quality
			std::sort(renderList.begin(), renderList.end(),
					  [=](std::tuple<uint8_t, float, glm::ivec2, uint8_t> A, std::tuple<uint8_t, float, glm::ivec2, uint8_t> B) -> bool
					  {
						  auto [objIdA, distanceA, positionA, lodLevelA] = A;
						  auto [objIdB, distanceB, positionB, lodLevelB] = B;

						  uint32_t lodExponentA;
						  lodExponentA = 1 << objects[objIdA]->lodLevels[lodLevelA];
						  uint32_t lodExponentB;
						  lodExponentB = 1 << objects[objIdB]->lodLevels[lodLevelB];
						  return (lodExponentA * distanceA) < (lodExponentB * distanceB);
					  });

			for (auto &candidate : renderList)
			{
				const auto [objId, distance, position, lodLevel] = candidate;
				if (lodLevel == objects[objId]->lodLevels.size() - 1) // nothing to improve
					continue;

				if (crtTriBudget - objects[objId]->getNumTriangles(lodLevel) + objects[objId]->getNumTriangles(lodLevel + 1) <= triangleBudget)
				{
					// We can afford this, increment LOD and repeat the outer cycle
					candidate = std::make_tuple(objId, distance, position, lodLevel + 1);
					improved = true;
					crtTriBudget = crtTriBudget - objects[objId]->getNumTriangles(lodLevel) + objects[objId]->getNumTriangles(lodLevel + 1);
					continue;
				}
			}
		}

		for (auto candidate : renderList)
		{
			const auto [objId, distance, position, lodLevel] = candidate;
#if 1 // Debug LODs at runtime
			if (lodLevel == 0)
			{
				basicProgram.setUniform4f("color", 0.85f, 0.15f, 0.15f, 1.0f);
			}
			else if (lodLevel == 1)
			{
				basicProgram.setUniform4f("color", 0.65f, 0.65f, 0.05f, 1.0f);
			}
			else if (lodLevel == 2)
			{
				basicProgram.setUniform4f("color", 0.75f, 0.15f, 0.65f, 1.0f);
			}
			else
			{
				basicProgram.setUniform4f("color", 0.75f, 0.70f, 0.85f, 1.0f);
			}
#else
			basicProgram.setUniform4f("color", 0.75f, 0.70f, 0.85f, 1.0f);
#endif

			modelView = camera.getModelViewMatrix();
			modelView = glm::translate(modelView, glm::vec3(position.x * 1.0f, 0.0f, position.y * 1.0f));
			basicProgram.setUniformMatrix4f("modelview", modelView);
			normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());
			basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
			objects[objId]->render(lodLevel);
		}

		// The code for rendering all models regardless of visibility, for comparison
		// for(int y = 0; y < tilemap.height; y++){
		// 	for(int x = 0; x < tilemap.width; x++){
		// 		bool statue = (tilemap.GetTile(x, y) > 0 && tilemap.GetTile(x, y) < 255);
		// 		if(statue){
		// 			for(int obj_id = 0;obj_id < objects.size();obj_id++){
		// 				if(object_codes[obj_id] == tilemap.GetTile(x, y)){
		// 					basicProgram.setUniform4f("color", 0.75f, 0.15f, 0.65f, 1.0f);
		// 					modelView = camera.getModelViewMatrix();
		// 					modelView = glm::translate(modelView, glm::vec3(x * 1.0f, 0.0f, y * 1.0f));
		// 					basicProgram.setUniformMatrix4f("modelview", modelView);
		// 					normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());
		// 					basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
		// 					objects[obj_id]->render();
		// 				}
		// 			}
		// 		}
		// 	}
		// }
	}
}

VectorCamera &Scene::getCamera()
{
	return camera;
}

// Load, compile, and link the vertex and fragment shader

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
	if (!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl
			 << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
	if (!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl
			 << endl;
	}
	basicProgram.init();
	basicProgram.addShader(vShader);
	basicProgram.addShader(fShader);
	basicProgram.link();
	if (!basicProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << basicProgram.log() << endl
			 << endl;
	}
	basicProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

// Render the room. Both the floor and the walls are instances of the
// same initial cube scaled and translated to build the room.

void Scene::renderRoom()
{
	glm::mat3 normalMatrix;
	glm::mat4 modelview;

	for (int y = -1; y <= tilemap.height; y++)
	{
		for (int x = -1; x <= tilemap.width; x++)
		{
			bool floor = (tilemap.GetTile(x, y) > 0);
			if (floor)
			{
				basicProgram.setUniform4f("color", 0.5f, 0.5f, 0.55f, 1.0f);
				modelview = camera.getModelViewMatrix();
				modelview = glm::translate(modelview, glm::vec3(x * 1.f, -0.5f, y * 1.f));
				modelview = glm::scale(modelview, glm::vec3(1.f, 1.f, 1.f));
				basicProgram.setUniformMatrix4f("modelview", modelview);
				normalMatrix = glm::inverseTranspose(modelview);
				basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
				cube->render();
			}
			else
			{
				int dx[4] = {0, -1, 0, 1};
				int dy[4] = {-1, 0, 1, 0};
				int floors = 0;
				for (int i = 0; i < 4; i++)
				{
					int nx = x + dx[i];
					int ny = y + dy[i];
					floors += (tilemap.GetTile(nx, ny) > 0);
				}
				if (true)
				{
					basicProgram.setUniform4f("color", 0.5f, 0.65f, 0.45f, 1.0f);
					modelview = camera.getModelViewMatrix();
					modelview = glm::translate(modelview, glm::vec3(x * 1.f, 0.f, y * 1.f));
					modelview = glm::scale(modelview, glm::vec3(1.f, 4.f, 1.f));
					basicProgram.setUniformMatrix4f("modelview", modelview);
					normalMatrix = glm::inverseTranspose(modelview);
					basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
					cube->render();
				}
			}
		}
	}
}

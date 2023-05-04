#include <iostream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Scene.h"
#include "PLYReader.h"


Scene::Scene()
{
	cube = NULL;
}

Scene::~Scene()
{
	if(cube != NULL)
		delete cube;
	for(TriangleMesh * mesh : objects){
		delete mesh;
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
}

// Loads the mesh into CPU memory and sends it to GPU memory (using GL)

bool Scene::loadMesh(const char *filename)
{
#pragma warning( push )
#pragma warning( disable : 4101)
	PLYReader reader;
#pragma warning( pop ) 

	TriangleMesh * mesh = new TriangleMesh();
	bool bSuccess = reader.readMesh(filename, *mesh);
	if(bSuccess)
	  mesh->sendToOpenGL(basicProgram);
	
	objects.push_back(mesh);
	return bSuccess;
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
}

// Render the scene. First the room, then the mesh it there is one loaded.

void Scene::render(uint8_t num_instances)
{
	glm::mat3 normalMatrix;

	basicProgram.use();
	basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
 	
 	renderRoom();
 	
	if(objects.size() > 0)
	{
		glm::mat4 modelView;
 		
		for(int y = 0; y < tilemap.height; y++){
			for(int x = 0; x < tilemap.width; x++){
				bool statue = (tilemap.GetTile(x, y) > 0 && tilemap.GetTile(x, y) < 255);
				if(statue){
					for(int obj_id = 0;obj_id < objects.size();obj_id++){
						if(object_codes[obj_id] == tilemap.GetTile(x, y)){
							basicProgram.setUniform4f("color", 0.75f, 0.15f, 0.65f, 1.0f);
							modelView = camera.getModelViewMatrix();
							modelView = glm::translate(modelView, glm::vec3(x * 1.0f, 0.0f, y * 1.0f));
							basicProgram.setUniformMatrix4f("modelview", modelView);
							normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());
							basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
							objects[obj_id]->render();
						}
					}
				}
			}	
		}
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
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	basicProgram.init();
	basicProgram.addShader(vShader);
	basicProgram.addShader(fShader);
	basicProgram.link();
	if(!basicProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << basicProgram.log() << endl << endl;
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

 	

	for(int y = -1; y <= tilemap.height; y++){
		for(int x = -1; x <= tilemap.width; x++){
			bool floor = (tilemap.GetTile(x, y) > 0);
			if(floor){
				basicProgram.setUniform4f("color", 0.5f, 0.5f, 0.55f, 1.0f);
				modelview = camera.getModelViewMatrix();
 				modelview = glm::translate(modelview, glm::vec3(x * 1.f, -0.5f, y * 1.f));
 				modelview = glm::scale(modelview, glm::vec3(1.f, 1.f, 1.f));
				basicProgram.setUniformMatrix4f("modelview", modelview);
				normalMatrix = glm::inverseTranspose(modelview);
				basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
				cube->render();
			}
			else{
				int dx[4] = {0, -1, 0, 1};
				int dy[4] = {-1, 0, 1, 0};
				int floors = 0;
				for(int i=0;i<4;i++){
					int nx = x + dx[i];
					int ny = y + dy[i];
					floors+=(tilemap.GetTile(nx, ny) > 0);
				}
				if(true){
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

 	// modelview = camera.getModelViewMatrix();
 	// modelview = glm::translate(modelview, glm::vec3(0.f, -0.5f, 0.f));
 	// modelview = glm::scale(modelview, glm::vec3(20.f, 1.f, 20.f));
	// basicProgram.setUniformMatrix4f("modelview", modelview);
	// normalMatrix = glm::inverseTranspose(modelview);
	// basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	// cube->render();
	
 	// modelview = camera.getModelViewMatrix();
 	// modelview = glm::translate(modelview, glm::vec3(0.f, 1.f, -9.5f));
 	// modelview = glm::scale(modelview, glm::vec3(20.f, 2.f, 1.f));
	// basicProgram.setUniformMatrix4f("modelview", modelview);
	// normalMatrix = glm::inverseTranspose(modelview);
	// basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	// cube->render();
	
 	// modelview = camera.getModelViewMatrix();
 	// modelview = glm::translate(modelview, glm::vec3(0.f, 1.f, 9.5f));
 	// modelview = glm::scale(modelview, glm::vec3(20.f, 2.f, 1.f));
	// basicProgram.setUniformMatrix4f("modelview", modelview);
	// normalMatrix = glm::inverseTranspose(modelview);
	// basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	// cube->render();
	
 	// modelview = camera.getModelViewMatrix();
 	// modelview = glm::translate(modelview, glm::vec3(-9.5f, 1.f, 0.f));
 	// modelview = glm::scale(modelview, glm::vec3(1.f, 2.f, 20.f));
	// basicProgram.setUniformMatrix4f("modelview", modelview);
	// normalMatrix = glm::inverseTranspose(modelview);
	// basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	// cube->render();
	
 	// modelview = camera.getModelViewMatrix();
 	// modelview = glm::translate(modelview, glm::vec3(9.5f, 1.f, 0.f));
 	// modelview = glm::scale(modelview, glm::vec3(1.f, 2.f, 20.f));
	// basicProgram.setUniformMatrix4f("modelview", modelview);
	// normalMatrix = glm::inverseTranspose(modelview);
	// basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	// cube->render();
}





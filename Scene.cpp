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
	mesh = NULL;
}

Scene::~Scene()
{
	if(cube != NULL)
		delete cube;
	if(mesh != NULL)
		delete mesh;
}


// Initialize the scene. This includes the cube we will use to render
// the floor and walls, as well as the camera.

void Scene::init()
{
	initShaders();
	cube = new TriangleMesh();
	cube->buildCube();
	cube->sendToOpenGL(basicProgram);
	currentTime = 0.0f;
	
	camera.init(glm::vec3(0.f, 0.5f, 2.f));
}

// Loads the mesh into CPU memory and sends it to GPU memory (using GL)

bool Scene::loadMesh(const char *filename)
{
#pragma warning( push )
#pragma warning( disable : 4101)
	PLYReader reader;
#pragma warning( pop ) 

	if(mesh != NULL)
	{
		mesh->free();
		delete mesh;
	}
	mesh = new TriangleMesh();
	bool bSuccess = reader.readMesh(filename, *mesh);
	if(bSuccess)
	  mesh->sendToOpenGL(basicProgram);
	
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
 	
	if(mesh != NULL)
	{
		basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
		glm::mat4 modelView;
		for (int x = 0; x < num_instances; x++){
			for (int y = 0; y < num_instances; y++){
				modelView = camera.getModelViewMatrix();
				modelView = glm::translate(modelView, glm::vec3(x * 1.0f, 0.0f, y * 1.0f));
				basicProgram.setUniformMatrix4f("modelview", modelView);
				normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());
				basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
				mesh->render();
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

 	basicProgram.setUniform4f("color", 0.5f, 0.5f, 0.55f, 1.0f);

 	modelview = camera.getModelViewMatrix();
 	modelview = glm::translate(modelview, glm::vec3(0.f, -0.5f, 0.f));
 	modelview = glm::scale(modelview, glm::vec3(20.f, 1.f, 20.f));
	basicProgram.setUniformMatrix4f("modelview", modelview);
	normalMatrix = glm::inverseTranspose(modelview);
	basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	cube->render();
	
 	modelview = camera.getModelViewMatrix();
 	modelview = glm::translate(modelview, glm::vec3(0.f, 1.f, -9.5f));
 	modelview = glm::scale(modelview, glm::vec3(20.f, 2.f, 1.f));
	basicProgram.setUniformMatrix4f("modelview", modelview);
	normalMatrix = glm::inverseTranspose(modelview);
	basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	cube->render();
	
 	modelview = camera.getModelViewMatrix();
 	modelview = glm::translate(modelview, glm::vec3(0.f, 1.f, 9.5f));
 	modelview = glm::scale(modelview, glm::vec3(20.f, 2.f, 1.f));
	basicProgram.setUniformMatrix4f("modelview", modelview);
	normalMatrix = glm::inverseTranspose(modelview);
	basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	cube->render();
	
 	modelview = camera.getModelViewMatrix();
 	modelview = glm::translate(modelview, glm::vec3(-9.5f, 1.f, 0.f));
 	modelview = glm::scale(modelview, glm::vec3(1.f, 2.f, 20.f));
	basicProgram.setUniformMatrix4f("modelview", modelview);
	normalMatrix = glm::inverseTranspose(modelview);
	basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	cube->render();
	
 	modelview = camera.getModelViewMatrix();
 	modelview = glm::translate(modelview, glm::vec3(9.5f, 1.f, 0.f));
 	modelview = glm::scale(modelview, glm::vec3(1.f, 2.f, 20.f));
	basicProgram.setUniformMatrix4f("modelview", modelview);
	normalMatrix = glm::inverseTranspose(modelview);
	basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	cube->render();
}





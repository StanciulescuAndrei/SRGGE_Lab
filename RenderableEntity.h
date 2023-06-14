#ifndef RENENT_H
#define RENENT_H

#include "TriangleMesh.h"
#include "ShaderProgram.h"
#include "PLYReader.h"
#include <vector>
#include <string>

class RenderableEntity{

public:
    std::vector<int> lodLevels = {6, 7, 9, 10};
    RenderableEntity(const char * path, uint8_t id, ShaderProgram &program){
        entityId = id;
        #pragma warning( push )
        #pragma warning( disable : 4101)
	    PLYReader reader;
        #pragma warning( pop ) 
        TriangleMesh * mesh;

        for(int level : lodLevels){
            mesh = new TriangleMesh();
            std::string fullPath = path;
            fullPath += "_LOD" + std::to_string(level) + ".ply";
            bool bSuccess = reader.readMesh(fullPath.c_str(), *mesh);
            if(bSuccess){
                mesh->sendToOpenGL(program);
                lods.push_back(mesh);
            }
        }
	    
        
    }

    uint32_t render(uint8_t lodLevel){
        lods[lodLevel]->render();
        return lods[lodLevel]->getTriangleCount();
    }

    uint32_t getNumTriangles(uint8_t lodLevel){
        return lods[lodLevel]->getTriangleCount();
    }



private:
    std::vector<TriangleMesh*> lods;
    uint8_t entityId;

};

#endif
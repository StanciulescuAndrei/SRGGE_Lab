#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "PLYReader.h"
#include <vector>
#include <queue>
#include "Octree.h"

using namespace std;

class Simplifier{
public:
    Simplifier(){}

    static Simplifier &instance(){
        static Simplifier s;
        return s;
    }

    bool loadMesh(const char* filename);
    bool computeLODs(int numLODs);
    bool writeSimplifications(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> faces, int level);

private:
    int numLODs;
    string output_folder;
    vector<glm::vec3> vertices;
    vector<glm::ivec3> faces;
    glm::vec3 bbox[2];
};


#endif
#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "PLYReader.h"
#include <vector>

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
    bool writeSimplifications();

private:
    int numLODs;
    char output_folder[256];
    vector<glm::vec3> vertices;
    vector<glm::ivec3> faces;
};


#endif
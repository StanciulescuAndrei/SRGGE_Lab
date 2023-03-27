#include "Simplifier.h"

bool Simplifier::loadMesh(const char* filename){
    vector<float> newVertices;
    vector<int> newFaces;

    PLYReader::readSimplified(filename, newVertices, newFaces);

    for(int i=0;i<newVertices.size();i+=3){
        Simplifier::vertices.push_back({newVertices[i], newVertices[i+1], newVertices[i+2]});
    }
    for(int i=0;i<newFaces.size();i+=3){
        Simplifier::faces.push_back({newFaces[i], newFaces[i+1], newFaces[i+2]});
    }
    
    return 0;
}

bool Simplifier::computeLODs(int numLODs){
    return 0;
}

bool Simplifier::writeSimplifications(){
    return 0;
}
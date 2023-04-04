#include "Octree.h"
#include <stdio.h>

bool insideBBox(glm::vec3 *bbox, glm::vec3 point){
    bool inside = true;
    for(int i=0;i<3;i++){
        inside = inside && (bbox[0][i] <= point[i]) && (bbox[1][i] > point[i]);
    }
    return inside;
}

void processNode(OctreeNode* currentNode, std::vector<glm::vec3>* vertices, int crt_depth, int max_depth){
    // No vertices inside node so no reason to split
    if(currentNode->verts_id.size() == 0){
        currentNode->isLeaf = true;
        return;
    }
    
    glm::vec3 halfsize = (currentNode->bbox[1] - currentNode->bbox[0]) * 0.5f;

    for (int i=0;i<8;i++){
        currentNode->children[i] = new OctreeNode();
        // Define node's BBox
        currentNode->children[i]->bbox[0][0] = currentNode->bbox[0][0] + ((i & 0b001)!=0) * halfsize[0];
        currentNode->children[i]->bbox[0][1] = currentNode->bbox[0][1] + ((i & 0b010)!=0) * halfsize[1];
        currentNode->children[i]->bbox[0][2] = currentNode->bbox[0][2] + ((i & 0b100)!=0) * halfsize[2];
        currentNode->children[i]->bbox[1] = currentNode->children[i]->bbox[0] + halfsize;
        // See which of the vertices go into the newly created node
        for(auto v : currentNode->verts_id){
            if(insideBBox(currentNode->children[i]->bbox, vertices->at(v))){
                currentNode->children[i]->verts_id.push_back(v);
            }
        }
        if(crt_depth < max_depth){
            currentNode->children[i]->isLeaf = false;
            processNode(currentNode->children[i], vertices, crt_depth+1, max_depth);
        }
        else
            currentNode->children[i]->isLeaf=true;
    }
}

void buildVertexLUT(OctreeNode* node, std::unordered_map<int, int>& lut, std::vector<glm::vec3>& octree_vertices, int crt_depth, int max_depth){
    if(node->verts_id.size() == 0){
        return;
    }
    if(crt_depth < max_depth){
        if(node->isLeaf){
            printf("[ERR] Cannot go deeper into the octree! Reached leaf at %i, requested depth %i\n", crt_depth, max_depth);
            return;
        }
        for (int i=0;i<8;i++){
            buildVertexLUT(node->children[i], lut, octree_vertices, crt_depth+1, max_depth);
        }
    }
    else if(crt_depth == max_depth){
        octree_vertices.push_back(node->bbox[0]);
        for(auto v : node->verts_id){
            lut[v] = current_node_id;
        }
        current_node_id += 1;
    }
}


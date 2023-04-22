#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>
#include <utility>
#include <eigen3/Eigen/Dense>
#include <iostream>

struct OctreeNode {
    OctreeNode * children[8] = {nullptr};
    std::vector<int> verts_id;
    glm::vec3 bbox[2];
    bool isLeaf;
};

extern int current_node_id;
extern int QEM_nodes;

bool insideBBox(glm::vec3* bbox, glm::vec3 point);

void processNode(OctreeNode* node, std::vector<glm::vec3>* vertices, int crt_depth, int max_depth);

void buildVertexLUT(OctreeNode* node, std::unordered_map<int, int>* lut, std::vector<glm::vec3>* octree_vertices, 
                    int crt_depth, int max_depth, std::vector<glm::vec3>* vertices,
                    std::vector<Eigen::Matrix4f>* error_metrics);

#endif
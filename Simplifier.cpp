#include "Simplifier.h"
#include <filesystem>
#include <iostream>

bool Simplifier::loadMesh(const char* filename){
    vector<float> newVertices;
    vector<int> newFaces;

    PLYReader::readSimplified(filename, newVertices, newFaces);
    Simplifier::bbox[0] = Simplifier::bbox[1] = {newVertices[0], newVertices[1], newVertices[2]};

    for(int i=0;i<newVertices.size();i+=3){
        Simplifier::vertices.push_back({newVertices[i], newVertices[i+1], newVertices[i+2]});

        // Compute AABB at the same time
        for(int j=0;j<3;j++){
            bbox[0][j] = min(bbox[0][j], newVertices[i+j]);
            bbox[1][j] = max(bbox[1][j], newVertices[i+j]);
        }
        
    }
    for(int i=0;i<newFaces.size();i+=3){
        Simplifier::faces.push_back({newFaces[i], newFaces[i+1], newFaces[i+2]});
    }

    // Transform mesh so it becomes enclosed in a unit cube from (0, 0, 0) -> (1, 1, 1);
    glm::vec3 scale = {bbox[1][0] - bbox[0][0], bbox[1][1] - bbox[0][1], bbox[1][2] - bbox[0][2]};
    for (auto &vertex : Simplifier::vertices){
        vertex = vertex - bbox[0];
        vertex = vertex / (scale);
    }

    Simplifier::output_folder = filename;

    return 0;
}

bool Simplifier::computeLODs(int numLODs){
    int maxOctreeDepth = 10;
    OctreeNode root;
    root.bbox[0] = glm::vec3(0.0);
    root.bbox[1] = glm::vec3(1.0);
    for(int i=0; i < Simplifier::vertices.size();i++){
        root.verts_id.push_back(i);
    }

    processNode(&root, &(Simplifier::vertices), 1, maxOctreeDepth);
    root.isLeaf = false;
    std::unordered_map<int, int> vertex_lookup;
    std::vector<glm::vec3> octree_vertices;

    current_node_id = 0;
    buildVertexLUT(&root, vertex_lookup, octree_vertices, 1, 9);

    vector<glm::ivec3> lod_faces;

    for(glm::ivec3 face : Simplifier::faces){
        int v0 = face[0]; int v1 = face[1]; int v2 = face[2];
        int tv0 = vertex_lookup[v0]; int tv1 = vertex_lookup[v1]; int tv2 = vertex_lookup[v2];

        if(tv0 == tv1 || tv1 == tv2 || tv0 == tv2){
            continue;
            // Discard triangle as it has become degenerate
        }

        lod_faces.push_back({tv0, tv1, tv2});
    }

    // Rescale to original
    glm::vec3 scale = {bbox[1][0] - bbox[0][0], bbox[1][1] - bbox[0][1], bbox[1][2] - bbox[0][2]};
    for (auto &vertex : octree_vertices){
        vertex = vertex * (scale);
        vertex = vertex + bbox[0];
        
    }

    printf("Writing simplified mesh...\n");
    writeSimplifications(octree_vertices, lod_faces, 0);
    


    return 0;
}

bool Simplifier::writeSimplifications(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> faces, int level){
    filesystem::path p(Simplifier::output_folder);
    string fpath = p.parent_path().string() + "/" + 
                   p.stem().string() + "_LOD"+
                   p.extension().string();
    std::cout << "Writing verts/faces to '" << fpath << "'..." << std::endl;
    int numVerts = vertices.size();
    int numFaces = faces.size();

    std::ofstream out(fpath);
    out << "ply\r\n"
        << "format ascii 1.0\r\n"
        << "element vertex " << numVerts << "\r\n"
        << "property float x\r\n"
        << "property float y\r\n"
        << "property float z\r\n"
        << "element face " << numFaces << "\r\n"
        << "property list uchar int vertex_index\r\n"
        << "end_header\r\n";

    // Output vertices
    for (const auto& v : vertices)
        out << v[0] << " " << v[1] << " " << v[2] << "\r\n";

    // Output faces
    for (const auto& f : faces)
        out << "3" << " " << f[0] << " " << f[1] << " " << f[2] << "\r\n";

    out << "\r\n";

    out.close();

    return 0;
}
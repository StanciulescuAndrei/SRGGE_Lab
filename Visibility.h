#ifndef VISIBILITY_H
#define VISIBILITY_H
#include <glm/glm.hpp>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <random>

#define _USE_MATH_DEFINES
#include <math.h>

#include "TileMap.h"

#define ray_eps 1e-5

class Visibility{
public:
    enum PlaneType{
        POS_X,
        POS_Y,
        NEG_X, NEG_Y
    };

    void computeVisibility(const char* outFile, TileMap & tilemap){
        printf("Computing cell visibility...\n");
        int mapSizeX = tilemap.width;
        int mapSizeY = tilemap.height;

        std::ofstream out(outFile);

        uint8_t * visibilityFlag;
        visibilityFlag = (uint8_t *)malloc(mapSizeX * mapSizeY * sizeof(uint8_t));

        std::default_random_engine generator;
        std::uniform_real_distribution<float> randDist(0.0 + ray_eps, 1.0 - ray_eps);

        std::vector<uint32_t> visibleCells;

        for (int x = 0; x < mapSizeX; x++)
        {
            for (int y = 0; y < mapSizeY; y++)
            {
                for(int i=0;i<mapSizeX * mapSizeY; i++){
                    visibilityFlag[i] = 0;
                }
                // Compute for each cell the visibility to other cells
                visibleCells.clear();
                for(float theta = 0; theta < 2 * M_PI; theta+=(2 * M_PI / 800)){
                    // Uniform structured sample of rays from the current cell

                    for(int samplePoint = 0; samplePoint < 100; samplePoint++){
                        // Uniform random sampling of points inside the cess from which to shoot the rays from

                        glm::vec2 startPos(x + randDist(generator), y + randDist(generator));
                        glm::vec2 rayDir = glm::normalize(glm::vec2(glm::sin(theta), glm::cos(theta)));

                        // Parameterize all "plane" (actually grid lines) intersections with ray as a function of t
                        // As any point on the ray is defined as P = startPos + t * rayDir
                        // We want to find the t's corresponding to the ray intersections with the cell walls
                        // Then, order them in ascending order and start traversing
                        
                        float t = 0.0f;

                        std::vector<std::pair<float, PlaneType>> intersections;

                        // X planes
                        if(glm::abs(rayDir.x) > ray_eps){
                            for (int dx = 0; dx < mapSizeX; dx++)
                            {
                                t = (dx - startPos.x) / rayDir.x;
                                bool positiveRayDirection = (rayDir.x >= 0);
                                bool positiveT = (t >= 0);
                                std::pair<float, PlaneType> intersection;
                                if(positiveRayDirection && positiveT){
                                    intersection = std::pair<float, PlaneType>(t, POS_X);
                                }
                                else{
                                    intersection = std::pair<float, PlaneType>(t, NEG_X);
                                }
                                intersections.push_back(intersection);
                            }
                        }

                        // Y planes
                        if(glm::abs(rayDir.y) > ray_eps){
                            for (int dy = 0; dy < mapSizeY; dy++)
                            {
                                t = (dy - startPos.y) / rayDir.y;
                                bool positiveRayDirection = (rayDir.y >= 0);
                                bool positiveT = (t >= 0);
                                std::pair<float, PlaneType> intersection;
                                if(positiveRayDirection && positiveT){
                                    intersection = std::pair<float, PlaneType>(t, POS_Y);
                                }
                                else{
                                    intersection = std::pair<float, PlaneType>(t, NEG_Y);
                                }
                                intersections.push_back(intersection);
                            }
                        }

                        std::sort(intersections.begin(), intersections.end(),
                                [](const std::pair<float, PlaneType> a, const std::pair<float, PlaneType> b) -> bool
                                {
                                    return a.first < b.first;
                                });
                        
                        // Walk through the intersections and compute visible cells
                        glm::ivec2 crt_pos(x, y);
                        visibilityFlag[crt_pos.x + crt_pos.y * mapSizeX] = 1;
                        for(auto intersection : intersections){
                            float t = intersection.first;
                            glm::vec2 candidate = startPos + t * rayDir;

                            // Only interested in the positive ray direction
                            if(t < 0)
                                continue;

                            // Out of bounds
                            if(candidate.x < 0 || candidate.y < 0 || candidate.x >= mapSizeX-ray_eps || candidate.y >= mapSizeY-ray_eps)
                                break;

                            switch(intersection.second){
                                case POS_X: crt_pos += glm::ivec2( 1,  0); break;
                                case NEG_X: crt_pos += glm::ivec2(-1,  0); break;
                                case POS_Y: crt_pos += glm::ivec2( 0,  1); break;
                                case NEG_Y: crt_pos += glm::ivec2( 0, -1); break;
                            }

                            // Out of bounds
                            if(crt_pos.x < 0 || crt_pos.y < 0 || crt_pos.x >= mapSizeX || crt_pos.y >= mapSizeY)
                                break;

                            
                            visibilityFlag[crt_pos.x + crt_pos.y * mapSizeX] = 1;

                            if(tilemap.GetTile(crt_pos.x, crt_pos.y) == 0)
                                break;
                        }
                    }
                }
                for(int i=0;i < mapSizeX * mapSizeY; i++){
                    out << (int)visibilityFlag[i] << " ";
                }
                out << std::endl;
            }
        }
        out.close();
        free(visibilityFlag);
        printf("Cell visibility done...\n");
    }

    Visibility(){
    }
    ~Visibility(){
    }
};

#endif
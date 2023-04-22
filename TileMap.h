#ifndef TILEMAP_H
#define TILEMAP_H
#include <stdlib.h>

// Class to store grayscale tilemap of the scene

class TileMap{
    public:
        int width, height;
        uint8_t* data;
        TileMap(){
            width = 0;
            height = 0;
            data = nullptr;
        }
        TileMap(uint8_t* _data, int _width, int _height, int _comp){
            data = (uint8_t*)malloc(_width * _height);
            width = _width;
            height = _height;
            // copy one component of the initial image
            for(int i = 0;i<width*height;i++){
                data[i] = _data[i * _comp];
            }
        }

        uint8_t GetTile(int x, int y){
            if(x < 0 || y < 0 || x >= width || y >= height)
                return 0;

            return data[y * width + x];
        }
};

#endif
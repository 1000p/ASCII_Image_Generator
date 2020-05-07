#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "SDL.h"
#include "SDL_Image.h"
#include "SDL_ttf.h"

class ASCIIBuilder
{
public:
    ASCIIBuilder(SDL_Surface* surf) 
    {
        init(surf);
    }

    ASCIIBuilder();

    void init(SDL_Surface* surf);

    void build();

    void close();

    void draw();

private:
    void drawColored(SDL_Rect& destination);

    void drawNoColor(SDL_Rect& destination);

    void resizeSurface();

    Uint32 get_pixel(int x, int y);

    char convertToChar(const Uint32& pixel);

    void convertCharsToTextures();

    void save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture);

    void createRendererAndWindow();

    std::unordered_map<char, SDL_Texture* > glyphsTextures;
    std::vector<char> charVec;
    std::vector<SDL_Color> pixelColors;
    std::vector<std::string> paintTable;
    SDL_Renderer* renderer;
    SDL_Window* drawWindow;
    SDL_Surface* image;
    SDL_PixelFormat* pFormat;

    float charMod;
    int tableSize;

    int width;
    int height;
    bool colored;


 

};


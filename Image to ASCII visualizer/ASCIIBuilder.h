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

    ASCIIBuilder() = default;

    void init(SDL_Surface* surf)
    {
        //Assign the image surface and paintTable
        image = surf;
        paintTable = { "``", "^^", "\"\"", ",,", "::", ";;", "II", "ll", "!!", "ii",
        "~~", "++", "__", "--", "??", "]]", "[[", "}}", "{{", "11", "))", "((",
         "||", "\\\\", "//", "tt", "ff", "jj", "rr", "xx", "nn", "uu", "vv", "cc",
         "zz", "XX", "YY", "UU", "JJ", "CC", "LL", "QQ", "00", "OO", "ZZ", "mm",
         "ww", "qq", "pp", "dd", "bb", "kk", "hh", "aa", "oo", "**", "##", "MM",
         "WW", "&&", "88", "%%", "BB", "@@", "$$", };

        tableSize = paintTable.size();

        //charMod is used to get index of element in the paintTable
        charMod = 255.0 / tableSize;
        //Reserve space in hash table
        glyphsTextures.reserve(tableSize);
        createRendererAndWindow();

        //Scale image to avoid enormous resolution
        resizeSurface();
        //Get the pixel format of the surface
        pFormat = image->format;

        //Reserve space for the tiling vector
        charVec.reserve(width * height);
        convertCharsToTextures();
    }

    void build();

    void close()
    {
        //Release resources

            //Clear texture hash table
        for (auto p : glyphsTextures)
        {
            SDL_DestroyTexture(p.second);
        }
        glyphsTextures.clear();

        //Destroy window and renderer
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(drawWindow);

    }

    void draw();

private:

    void resizeSurface();

    Uint32 get_pixel(int x, int y);

    char convertToChar(const Uint32& pixel);

    void convertCharsToTextures();

    void save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture);

    void createRendererAndWindow();

    std::vector<char> charVec;
    std::unordered_map<char, SDL_Texture* > glyphsTextures;
    std::vector<std::string> paintTable;
    SDL_Renderer* renderer;
    SDL_Window* drawWindow;
    SDL_Surface* image;
    SDL_PixelFormat* pFormat;

    int tableSize;

    int width;
    int height;


    float charMod;

};


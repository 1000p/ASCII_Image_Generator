#include "SDL.h"
#include "SDL_Image.h"

#include "ASCIIBuilder.h"

#include <fstream>
#include <sstream>
#include <iostream>

std::string getImagePath()
{
    std::string path;
    getline(std::cin, path);
    return path;
}


int main(int argc, char* argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    std::string imagePath = getImagePath();

    std::cout << imagePath << "\n";

    SDL_Surface* imageOriginal = IMG_Load(imagePath.c_str());


    //Construct, build and draw
    ASCIIBuilder builder(imageOriginal);
    builder.build();
    builder.draw();

    //Main loop flag
    bool quit = false;

    //Event handler
    SDL_Event e;

    //While application is running
    while (!quit)
    {
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            //User requests quit
            if (e.type == SDL_QUIT)
            {
                quit = true;
                builder.close();
            }
        }
    }


    return 0;
}
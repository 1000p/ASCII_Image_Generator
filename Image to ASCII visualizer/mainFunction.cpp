#include "SDL.h"
#include "SDL_Image.h"

#include "ASCIIBuilder.h"

#include <fstream>
#include <sstream>
#include <iostream>

void terminateProgram(const std::string& errorMsg, int ErrorCode)
{
    std::cout << "The program terminated! Exit message: " << errorMsg << std::endl;
    exit(ErrorCode);
}

std::string getImagePath()
{
    std::cout << "Enter absolute path to the image:\n";
    std::string path;
    getline(std::cin, path);
    return path;
}

SDL_Surface* tryLoad(std::string& path)
{
    static int tryCounter = 3;
    SDL_Surface* image = IMG_Load(path.c_str());
    if (image == nullptr)
    {
        std::cout << "Error loading the image with path: " << path << "\n" <<
            "Please try again.\nYou left with " << tryCounter << " number of tries"
            " before the program terminates!" << std::endl;
        --tryCounter;
        if (tryCounter < 0)
        {
            terminateProgram("You have reached zero number of tries! \n", -404);
        }

        return nullptr;
    }
    return image;
}


int main(int argc, char* argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    std::string imagePath = getImagePath();

    std::cout <<"You entered this image path: "<< imagePath << "\n";

    ASCIIBuilder builder;
    SDL_Surface* image = tryLoad(imagePath);

    while (!image)
    {
        imagePath = getImagePath();
        image = tryLoad(imagePath);
    }
    //Initialize
    builder.init(image);
    
    //Build and draw
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
                terminateProgram("You have exited the program.\n"
                    "An image was generated in the program installation folder "
                    "with a name: image.png", 0);
            }
        }
    }


    return 0;
}
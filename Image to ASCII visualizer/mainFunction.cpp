#include "UtilityFunctions.h"
#include "ASCIIBuilder.h"

int main(int argc, char* argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    std::string imagePath = getImagePath();

    std::cout << "You entered this image path: " << imagePath << std::endl;

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
#include "UtilityFunctions.h"

void terminateProgram(const std::string& errorMsg, int ErrorCode)
{
	std::cout << "The program terminated! Exit message: " << errorMsg << std::endl;
	exit(ErrorCode);
}

std::string getImagePath()
{
	std::cout << "Enter absolute path to the image and image format"
        "(.png, .jpg, .bmp, etc.):" << std::endl;;
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

#pragma once

#include "SDL.h"
#include "SDL_Image.h"

#include <iostream>
#include <sstream>

void terminateProgram(const std::string& errorMsg, int ErrorCode);

std::string getImagePath();

SDL_Surface* tryLoad(std::string& path);
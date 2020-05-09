#include "ASCIIBuilder.h"
#include "UtilityFunctions.h"

ASCIIBuilder::ASCIIBuilder() :renderer(nullptr), drawWindow(nullptr),
image(nullptr), colored(false)
{}

void ASCIIBuilder::init(SDL_Surface* surf)
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
    pFormat = *image->format;

    //Reserve space for the tiling vector
    charVec.reserve(width * height);
    convertCharsToTextures();

    SDL_HideWindow(drawWindow);

    //Should color the image ?
    char confirm;
    std::cout << "\nCreate colored image? Press Y for YES and any other key for NO.\nY/N?";
    std::cin >> confirm;
    if (confirm == 'Y' || confirm == 'y')
    {
        colored = true;
        pixelColors.reserve(charVec.size());
    }

}

void ASCIIBuilder::build()
{
    for (int row = 0; row < this->height; ++row)
    {
        for (int col = 0; col < this->width; ++col)
        {
            char emp{ convertToChar(get_pixel(col, row)) };
            charVec.emplace_back(emp);
        }
        charVec.emplace_back('\n');
    }

    //Release image resources 
    SDL_FreeSurface(image);
}

void ASCIIBuilder::close()
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

void ASCIIBuilder::resizeSurface()
{
    //In order to renderer to work correctly
    SDL_ShowWindow(drawWindow);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    //Convert surface to texture
    SDL_Texture* original = SDL_CreateTextureFromSurface(renderer, image);

    //Free surface to release memory and get it's format for later in the function
    pFormat = *image->format;
    SDL_FreeSurface(image);

    Uint32 format;
    int W;
    int H;
    //Get texture data and set widht and height
    SDL_QueryTexture(original, &format, NULL, &W, &H);

    //Restrict output image to 8k
    int maxPixelOnVertice = 8192;

    //Get modifier from division of 13 (because that's how "big" one "symbol pixel" is)
    int modifier = 8192 / 13;

    
    //Resize according to aspect ratio
    float aspectRatio;
    if (W > modifier || H > modifier)
    {
        if (W > H)
        {
            aspectRatio = W / (H * 1.0);
            H = modifier / aspectRatio;
            W = H * aspectRatio;
        }
        else
        {
            aspectRatio = H / (W * 1.0);
            W = modifier / aspectRatio;
            H = W * aspectRatio;
        }
    }

    width = W; height = H;

    //Create target texture to render a resized version
    SDL_Texture* resized = SDL_CreateTexture(renderer, format,
        SDL_TEXTUREACCESS_TARGET, W, H);

    //Set target,render image
    SDL_SetRenderTarget(renderer, resized);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, original, NULL, NULL);   

    SDL_SetRenderTarget(renderer, resized);
    //Create resized surface
    SDL_Surface* surface = SDL_CreateRGBSurface(0, W, H,pFormat.BitsPerPixel , 0, 0, 0, 0);

    //Read texture to surface
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels,
        surface->pitch);
    //Return to default render target
    SDL_SetRenderTarget(renderer, NULL);

    //Realese memory
    SDL_DestroyTexture(original);
    SDL_DestroyTexture(resized);
 
    image = surface;

}

Uint32 ASCIIBuilder::get_pixel(int x, int y)
{
    int bpp = pFormat.BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8* p = (Uint8*)image->pixels + y * image->pitch + x * bpp;

    switch (bpp)
    {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16*)p;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32*)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}



char ASCIIBuilder::convertToChar(const Uint32& pixel)
{
    SDL_Color RGB;
    SDL_GetRGB(pixel, &pFormat, &RGB.r, &RGB.g, &RGB.b);
    double brightness = (RGB.r + RGB.g + RGB.b) / 3.0;
    int it;

    it = (int(brightness / charMod) >= tableSize) ? tableSize - 1 : int(brightness / charMod);
    char toRet = paintTable[it][0];

    if (colored)
    {
        pixelColors.push_back(RGB);
    }

    return toRet;
}

void ASCIIBuilder::convertCharsToTextures()
{
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("Font.ttf", 12);//The best font size to use
    SDL_Color color{ 255,255,255,255 };

    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderClear(renderer);

    //Create surfaces from characters
    std::vector<SDL_Surface*> strSurfaces;
    strSurfaces.reserve(tableSize);
    for (auto s : paintTable)
    {
        SDL_Surface* surf = TTF_RenderText_Solid(font, s.c_str(), color);
        strSurfaces.push_back(surf);
    }

    //Create textures from surfaces
    std::vector<SDL_Texture*> strTextures;
    strTextures.reserve(tableSize);
    for (auto s : strSurfaces)
    {
        SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, s);
        strTextures.push_back(text);
    }

    //Data for optimized texture
    Uint32 format;
    int W;
    int H;
    SDL_Rect dst{ 0,0,0,0 };

    //Create optimized/square textures from text textures
    int it = 0;
    for (auto t : strTextures)
    {
        //Get data from old texture
        SDL_QueryTexture(t, &format, NULL, &W, &H);
        dst.w = W; dst.h = H;
        //Create square texture, render old texture above,
            //present and save tohash table
        SDL_Texture* newT = SDL_CreateTexture(renderer, format,
            SDL_TEXTUREACCESS_TARGET, 13, 13); //13 x 13 because it scales well
        SDL_SetRenderTarget(renderer, newT);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, t, NULL, NULL);
        SDL_RenderPresent(renderer);

        glyphsTextures[paintTable[it][0]] = newT;
        ++it;
    }
    //Release resources
        //Release memory from old textures
    for (auto t : strTextures)
    {
        SDL_DestroyTexture(t);
    }
    //Release memory from old surfaces
    for (auto s : strSurfaces)
    {
        SDL_FreeSurface(s);
    }
    //Release FONT
    TTF_CloseFont(font);

}

void ASCIIBuilder::draw()
{
    //In order to renderer to work correctly
    SDL_ShowWindow(drawWindow);

    //Get data from a tile and save it
    int W;
    int H;
    Uint32 format;
    SDL_QueryTexture(glyphsTextures.begin()->second,
        &format, NULL, &W, &H);

    //Our output texture
    SDL_Texture* outText = SDL_CreateTexture(renderer,
        format, SDL_TEXTUREACCESS_TARGET, width * W, height * H);

    //Render to texture
    SDL_SetRenderTarget(renderer, outText);
    SDL_RenderClear(renderer);

    //Destination for rendering
    SDL_Rect dst = { 0, 0, W, H };

    if (colored)
    {
        drawColored(dst);
    }
    else
    {
        drawNoColor(dst);
    }


    //Present to output texture
    SDL_RenderPresent(renderer);
    //Open preview window, target it with renderer,
        //Clear the window and render output texture above - present
    SDL_ShowWindow(drawWindow);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, outText, NULL, NULL);
    SDL_RenderPresent(renderer);

    //Check for errors
    if (outText)
    {
        std::cout << "Draw function ended successfully!" << std::endl;
    }
    else
    {
        std::cout << "Error with texture generation in draw function!\n";
        std::string outMsg = "Terminating program! If you receive this message again,"
            "contact the developer with the exit code. SDL_Error: ";
        outMsg.append(SDL_GetError()).append("\n");
        terminateProgram(outMsg, -101);
    }

    //Save output texture to a file
    save_texture("image.png", renderer, outText);

    //Release resources
    SDL_DestroyTexture(outText);
    charVec.clear();
    //Release the vector of chars
    charVec.clear();

}

void ASCIIBuilder::drawColored(SDL_Rect& destination)
{
    int W = destination.w;
    int H = destination.h;

    int it = 0;
    for (auto c : charVec)
    {
        SDL_Texture* texP = glyphsTextures[c];
        SDL_Color* colorP;

        //Still printing a row
        if (c != '\n')
        {
            colorP = &pixelColors[it];
            SDL_SetTextureColorMod(texP, colorP->r, colorP->g, colorP->b);
            SDL_RenderCopy(renderer, texP, NULL, &destination);
            destination.x += W;
            ++it;
        }
        //We hit a row end move to a new row
        else
        {
            destination.x = 0;
            destination.y += H;
        }

    }
}

void ASCIIBuilder::drawNoColor(SDL_Rect& destination)
{
    int W = destination.w;
    int H = destination.h;

    for (auto c : charVec)
    {
        //Still printing a row
        if (c != '\n')
        {
            SDL_RenderCopy(renderer, glyphsTextures[c], NULL, &destination);

            destination.x += W;
        }
        //We hit a row end move to a new row
        else
        {
            destination.x = 0;
            destination.y += H;
        }
    }
}

void ASCIIBuilder::save_texture(const char* file_name, SDL_Renderer* renderer, SDL_Texture* texture)
{
    //Renderer old target
    SDL_Texture* target = SDL_GetRenderTarget(renderer);

    //Target new texture, get texture data
    SDL_SetRenderTarget(renderer, texture);
    int W, H;
    SDL_QueryTexture(texture, NULL, NULL, &W, &H);
 

    //Create new surface and read texture pixel data on it
    SDL_Surface* surface = SDL_CreateRGBSurface(0, W, H,pFormat.BitsPerPixel, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);

    //Realease resources and target old target
    std::cout << "\nSaving image, please wait.\nYou can close the program when it's done.\n";
    IMG_SavePNG(surface, file_name);
    std::cout << "\nImage saved! You can now close the program!" << std::endl;
    
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
}

void ASCIIBuilder::createRendererAndWindow()
{
    //Create window
    drawWindow = SDL_CreateWindow("Visualizer",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_HIDDEN);
    if (drawWindow == NULL)
    {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
    }
    else
    {
        //Create renderer for window
        renderer = SDL_CreateRenderer(drawWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
        if (renderer == NULL)
        {
            printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        }
        else
        {
            //Initialize renderer color
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
            SDL_RenderClear(renderer);
        }
    }
}
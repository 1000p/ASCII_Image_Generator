#include "ASCIIBuilder.h"

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


    //REMOVED FROM HERE
}

void ASCIIBuilder::resizeSurface()
{

    SDL_ShowWindow(drawWindow);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    //Convert surface to texture
    SDL_Texture* original = SDL_CreateTextureFromSurface(renderer, image);
    Uint32 format;
    int W;
    int H;
    //Get texture data and set widht and height
    SDL_QueryTexture(original, &format, NULL, &W, &H);


    //Resize according to aspect ratio
    float aspectRatio = W / (H * 1.0);
    if (W > 680 || H > 680)
    {
        if (W > H)
        {
            H = 680 / aspectRatio;
            W = H * aspectRatio;
        }
        else
        {
            W = 680 / aspectRatio;
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
    SDL_Surface* surface = SDL_CreateRGBSurface(0, W, H, 32, 0, 0, 0, 0);

    //Read texture to surface
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels,
        surface->pitch);
    //Return to default render target
    SDL_SetRenderTarget(renderer, NULL);

    //Realese memory
    SDL_DestroyTexture(original);
    SDL_DestroyTexture(resized);
    SDL_FreeSurface(image);
    image = surface;

}

Uint32 ASCIIBuilder::get_pixel(int x, int y)
{
    int bpp = pFormat->BytesPerPixel;
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
    SDL_GetRGB(pixel, pFormat, &RGB.r, &RGB.g, &RGB.b);
    double brightness = (RGB.r + RGB.g + RGB.b) / 3.0;
    int it;

    it = (int(brightness / charMod) >= tableSize) ? tableSize - 1 : int(brightness / charMod);
    char toRet = paintTable[it][0];

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

    for (auto c : charVec)
    {
        //Still printing a row
        if (c != '\n')
        {
            SDL_RenderCopy(renderer, glyphsTextures[c], NULL, &dst);

            dst.x += W;
        }
        //We hit a row end move to a new row
        else
        {
            dst.x = 0;
            dst.y += H;
            SDL_RenderCopy(renderer, glyphsTextures[c], NULL, &dst);
        }
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

    //Save output texture to a file
    save_texture("image.png", renderer, outText);

    //Release resources
    SDL_DestroyTexture(outText);
    charVec.clear();
    //Release the vector of chars
    charVec.clear();

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
    SDL_Surface* surface = SDL_CreateRGBSurface(0, W, H, 32, 0, 0, 0, 0);
    SDL_RenderReadPixels(renderer, NULL, surface->format->format, surface->pixels, surface->pitch);

    //Realease resources and target old target
    IMG_SavePNG(surface, file_name);
    SDL_FreeSurface(surface);
    SDL_SetRenderTarget(renderer, target);
} 

 void ASCIIBuilder::createRendererAndWindow()
{
    //Create window
    drawWindow = SDL_CreateWindow("SDL Tutorial",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1152, 864, SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_HIDDEN);
    if (drawWindow == NULL)
    {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
    }
    else
    {
        //Create renderer for window
        renderer = SDL_CreateRenderer(drawWindow, -1, SDL_RENDERER_ACCELERATED);
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
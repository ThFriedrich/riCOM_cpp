/* Copyright (C) 2021 Thomas Friedrich, Chu-Ping Yu,
 * University of Antwerp - All Rights Reserved.
 * You may use, distribute and modify
 * this code under the terms of the GPL3 license.
 * You should have received a copy of the GPL3 license with
 * this file. If not, please visit:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Authors:
 *   Thomas Friedrich <thomas.friedrich@uantwerpen.be>
 *   Chu-Ping Yu <chu-ping.yu@uantwerpen.be>
 */

#include "SdlImageWindow.h"

SdlImageWindow::SdlImageWindow(const char *title, SDL_Surface *srf, int nx, int ny, float scale)
{
    this->title = title;
    create_window(srf, nx, ny, scale);
};

SdlImageWindow::SdlImageWindow(const char *title)
{
    this->title = title;
    this->window = NULL;   // Pointer for the window
    this->renderer = NULL; // Pointer for the renderer
    this->tex = NULL;      // Texture for the window
    this->srf = NULL;      // Surface (data source)
};

void SdlImageWindow::create_window(SDL_Surface *srf, int nx, int ny, float scale)
{
    // Creating window
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scale * nx, scale * ny, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    // Creating Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    // Creating texture for hardware rendering
    tex = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_STATIC, nx, ny);
    if (tex == NULL)
    {
        std::cout << "Texture could not be created! SDL Error: " << SDL_GetError() << std::endl;
    }
    // Maintain Pixel aspect ratio on resizing
    if (SDL_RenderSetLogicalSize(renderer, scale * nx, scale * ny) < 0)
    {
        std::cout << "Logical size could not be set! SDL Error: " << SDL_GetError() << std::endl;
    }
    this->srf = srf;
}

void SdlImageWindow::update_image()
{
    if (srf != NULL)
    {
        if (SDL_UpdateTexture(tex, NULL, srf->pixels, srf->pitch) == -1)
        {
            std::cout << "SDL_UpdateTexture failed: " << SDL_GetError() << std::endl;
        }
        if (SDL_RenderCopy(renderer, tex, NULL, NULL) == -1)
        {
            std::cout << "SDL_RenderCopy failed: " << SDL_GetError() << std::endl;
        }
        SDL_RenderPresent(renderer);
    }
}

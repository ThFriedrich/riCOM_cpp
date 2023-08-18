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

#ifndef SDLIMAGEWINDOW_H
#define SDLIMAGEWINDOW_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <iostream>

class SdlImageWindow
{
public:
    const char *title; // Window title
    explicit SdlImageWindow(const char *title);
    explicit SdlImageWindow(const char *title, SDL_Surface *srf, int nx, int ny, float scale);
    void create_window(SDL_Surface *srf, int nx, int ny, float scale);
    void update_image();

private:
    SDL_Window *window;     // Pointer for the window
    SDL_Renderer *renderer; // Pointer for the renderer
    SDL_Texture *tex;       // Texture for the window
    SDL_Surface *srf;       // Surface (data source)
};
#endif // SDLIMAGEWINDOW_H
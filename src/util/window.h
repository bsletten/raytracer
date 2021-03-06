/* MIT License
Copyright (c) 2018 Biro Eniko
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <SDL2/SDL.h>

#include "util/camera.h"
#include "util/renderer.h"
#include "util/image.h"

class LParams;

struct Window
{

    int nx, ny;
    float thetaInit, phiInit;
    float zoomScale, stepScale;

    // x, y, w, h
    SDL_Rect SDLWindowRect;
    SDL_Window* SDLWindow;
    SDL_Renderer* SDLRenderer;
    SDL_Texture* SDLTexture;

    bool quit;
    bool mouseDragIsInProgress;
    bool refresh;

	float theta;
	float phi;
    const float delta = 0.1f * static_cast<float>(M_PI) / 180.0f;

    Camera* windowCamera;
    Renderer* windowRenderer;

    CUDA_HOST Window(const std::unique_ptr<Camera>& cam,
                     const std::unique_ptr<Renderer>& renderer,
                     const int nx, const int ny,
                     const float thetaInit, const float phiInit,
                     const float zoomScale, const float stepScale) :
                     nx(nx), ny(ny),
                     thetaInit(thetaInit), phiInit(phiInit),
                     zoomScale(zoomScale),
                     stepScale(stepScale),
                     windowCamera(cam.get()),
                     windowRenderer(renderer.get())
    {
        SDLWindowRect = { 0, 0, nx, ny };
	    theta = thetaInit;
	    phi = phiInit;

        quit = false;
        mouseDragIsInProgress = false;
        refresh = false;

        SDLWindow = nullptr;
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            #ifndef CUDA_ENABLED
                std::cout << "SDL could not initialize! SDL_Error: %s\n" <<  SDL_GetError() << std::endl;
            #endif
        }
        else
        { 
            SDLWindow = SDL_CreateWindow("Ray tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, nx, ny, SDL_WINDOW_SHOWN); 
            if (SDLWindow == nullptr)
            { 
                #ifndef CUDA_ENABLED
                    std::cout << "Window could not be created! SDL_Error: %s\n" <<  SDL_GetError() << std::endl;;
                #endif
            }
            SDLRenderer = SDL_CreateRenderer(SDLWindow, -1, SDL_RENDERER_SOFTWARE);
            if (SDLRenderer == nullptr)
            {
                #ifndef CUDA_ENABLED
                    std::cout << "Renderer could not be created! SDL_Error: %s\n" <<  SDL_GetError() << std::endl;;
                #endif
            }
        }

        SDL_RenderSetLogicalSize(SDLRenderer, SDLWindowRect.w, SDLWindowRect.h);
        SDL_SetRenderDrawColor(SDLRenderer, 0, 0, 0, 255);
        SDL_RenderClear(SDLRenderer);
        SDL_RenderPresent(SDLRenderer);

        SDLTexture = SDL_CreateTexture(SDLRenderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STATIC,
                                    nx, ny);

        windowCamera->rotate(theta, phi);
    }

    CUDA_HOSTDEV ~Window()
    {
        SDL_DestroyTexture(SDLTexture);
        SDL_DestroyRenderer(SDLRenderer);
        SDL_DestroyWindow(SDLWindow); 
        SDL_Quit();
    }

    CUDA_HOSTDEV void updateImage(LParams& LParams,
                                  RParams& RParams,
                                  int sampleCount);

    CUDA_HOSTDEV void pollEvents(std::unique_ptr<Image>& image)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_MOUSEMOTION:
                    if (mouseDragIsInProgress)
                    {
                        int mx = event.motion.xrel;
                        int my = event.motion.yrel;
					    theta += -my * delta;
					    if (theta < delta) 
                            theta = delta;
                        if (theta > (static_cast<float>(M_PI_2) - delta))
                            theta = static_cast<float>(M_PI_2) - delta;
					    phi += -mx * delta;
					    windowCamera->rotate(theta, phi);

                        image->resetImage();

                        refresh = true;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    {
				        mouseDragIsInProgress = true;
                    }
                    break;
			    case SDL_MOUSEBUTTONUP:
                    {
				        mouseDragIsInProgress = false;
                    }
				    break;
                case SDL_MOUSEWHEEL:
                    {
                        if(event.wheel.y > 0) // scroll up
                        {
                            windowCamera->zoom(-zoomScale);
                        }
                        else if(event.wheel.y < 0) // scroll down
                        {
                            windowCamera->zoom(+zoomScale);
                        }
                        
                        image->resetImage();

                        refresh = true;
                    }
                    break;
                case SDL_KEYDOWN:
                    {
                        switch( event.key.keysym.sym )
                        {
                            case SDLK_UP:
                                windowCamera->translate(FORWARD, stepScale);
                            break;

                            case SDLK_DOWN:
                                windowCamera->translate(BACKWARD, stepScale);
                            break;

                            case SDLK_LEFT:
                                windowCamera->translate(LEFT, stepScale);
                            break;

                            case SDLK_RIGHT:
                                windowCamera->translate(RIGHT, stepScale);
                            break;

                            default:
                                return;
                        }

                        image->resetImage();

                        refresh = true;
                    }
                    break;
			    case SDL_QUIT:
				    quit = true;
				    break;
            }
        }

    }

    CUDA_HOSTDEV void moveCamera(std::unique_ptr<Image>& image, uint8_t *fileOutputImage)
    {

        theta += -1.0f * delta;
        if (theta < delta)
            theta = delta;
        if (theta > (static_cast<float>(M_PI_2) - delta))
            theta = static_cast<float>(M_PI_2) - delta;
        phi += -10.0f * delta;
        windowCamera->rotate(theta, phi);

        image->resetImage();

        refresh = true;

    }

    CUDA_HOSTDEV void waitQuit()
    {

        SDL_Event event;
        while (!quit)
        {
            SDL_WaitEvent(&event);
            quit = (event.type == SDL_QUIT);
        }

    }
};

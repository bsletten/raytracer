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

#include <iostream>
#include <random>
#include <float.h>
#include <omp.h>
#include <memory>

#include "hitables/hitablelist.h"
#include "util/camera.h"
#include "util/image.h"
#include "util/randomgenerator.h"
#include "materials/material.h"
#include "hitables/sphere.h"

class RParams;

class Renderer
{
    bool showWindow;
    bool writeImagePPM;
    bool writeImagePNG;

    public:
        CUDA_HOSTDEV Renderer(bool showWindow,
                              bool writeImagePPM,
                              bool writeImagePNG) :
                              showWindow(showWindow),
                              writeImagePPM(writeImagePPM),
                              writeImagePNG(writeImagePNG)
        {

        }

        CUDA_DEV Vec3 color(RandomGenerator& rng,
                            const Ray& r,
                            Hitable* world,
                            int depth)
        {

            Ray curRay = r;
            Vec3 curAttenuation = Vec3(1.0f, 1.0f, 1.0f);
            for (int i = 0; i < 50; i++)
            {
                HitRecord rec;
                if (world->hit(curRay, 0.001f, FLT_MAX, rec))
                {
                    Ray scattered;
                    Vec3 attenuation;
                    if (rec.matPtr->scatter(rng, curRay, rec, attenuation, scattered))
                    {
                        curAttenuation *= attenuation;
                        curRay = scattered;
                    }
                    else
                        return Vec3(0.0f, 0.0f, 0.0f);
                }
                else
                {
                    Vec3 unit_direction = unitVector(curRay.direction());
                    float t = 0.5f * (unit_direction.y() + 1.0f);
                    Vec3 c = (1.0f-t) * Vec3(1.0f, 1.0f, 1.0f) + t*Vec3(0.5f, 0.7f, 1.0f);
                    return curAttenuation * c;
                }
            }
            return Vec3(0.0f, 0.0f, 0.0f); // Exceeded recursion

        }

        CUDA_HOSTDEV bool traceRays(RParams& RParams,
                                    int sampleCount);

        #ifdef CUDA_ENABLED
            void traceRaysCuda(RParams& RParams,
                               int sampleCount);
        #else
            CUDA_HOSTDEV void render(int i, int j,
                                     RParams& rParams,
                                     int sampleCount);
            CUDA_HOSTDEV void display(int i, int j,
                                      std::unique_ptr<Image>& image);
        #endif // CUDA_ENABLED
};

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

#include "util/vec3.h"
#include "materials/perlin.h"

class Texture
{

    public:

        CUDA_DEV virtual Vec3 value(float u, float v, const Vec3& p) const = 0;

};

class ConstantTexture : public Texture
{

    public:

        CUDA_DEV ConstantTexture() { }
        CUDA_DEV ConstantTexture(Vec3 c) : color(c) { }

        CUDA_DEV virtual Vec3 value(float u, float v, const Vec3& p) const
        {
            return color;
        }

        Vec3 color;

};

class CheckerTexture : public Texture
{

    public:

        CUDA_DEV CheckerTexture() { }
        CUDA_DEV CheckerTexture(Texture *t0, Texture *t1): even(t0), odd(t1) { }

        CUDA_DEV virtual Vec3 value(float u, float v, const Vec3& p) const
        {
            float sines = sin(10*p.x())*sin(10*p.y())*sin(10*p.z());
            if (sines < 0)
                return odd->value(u, v, p);
            else
                return even->value(u, v, p);
        }

        Texture *odd;
        Texture *even;

};

class NoiseTexture : public Texture
{

    public:

        CUDA_DEV NoiseTexture() {}
        CUDA_DEV NoiseTexture(float sc) : scale(sc) {}

        CUDA_DEV virtual Vec3 value(float u, float v, const Vec3& p) const
        {
            return Vec3(1,1,1)*0.5*(1 + sin(scale*p.x() + 5*noise.turb(scale*p)));
        }

        Perlin noise;
        float scale;

};

class ImageTexture : public Texture
{

    public:

        CUDA_DEV ImageTexture() {}
        CUDA_DEV ImageTexture(unsigned char *pixels, int A, int B) :
            data(pixels), nx(A), ny(B)
        {

        }

        CUDA_DEV virtual Vec3 value(float u, float v, const Vec3& p) const;

        unsigned char *data;
        int nx, ny;

};

inline Vec3 ImageTexture::value(float u, float v, const Vec3& p) const
{
     int i = u*nx;
     int j = (1 - v) * ny - 0.001f;
     if (i < 0)
         i = 0;
     if (j < 0)
         j = 0;
     if (i > nx-1)
         i = nx-1;
     if (j > ny-1)
         j = ny-1;
     float r = int(data[3*i + 3*nx*j]) / 255.0f;
     float g = int(data[3*i + 3*nx*j+1]) / 255.0f;
     float b = int(data[3*i + 3*nx*j+2]) / 255.0f;
     return Vec3(r, g, b);
}


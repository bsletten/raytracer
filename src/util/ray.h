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

class Ray
{

        Vec3 A;             // ray origin
        Vec3 B;             // ray direction
        float m_time;

    public:

        CUDA_HOSTDEV Ray() {}
        CUDA_HOSTDEV Ray(const Vec3& a, const Vec3& b, float ti = 0.0f)
        {
            A = a;
            B = b;
            m_time = ti;
        }

        CUDA_HOSTDEV Vec3 origin() const                     { return A; }
        CUDA_HOSTDEV Vec3 direction() const                  { return B; }
        CUDA_HOSTDEV float time() const                      { return m_time; }
        CUDA_HOSTDEV Vec3 pointAtParameter(float t) const    { return A + t*B; }

};

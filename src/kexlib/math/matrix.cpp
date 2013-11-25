//
// Copyright (c) 2013-2014 Samuel Villarreal
// svkaiser@gmail.com
// 
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 
 //   2. Altered source versions must be plainly marked as such, and must not be
 //   misrepresented as being the original software.
// 
//    3. This notice may not be removed or altered from any source
//    distribution.
// 
//-----------------------------------------------------------------------------
//
// DESCRIPTION: Matrix (left handed) operations
//
// Reference:
// _________________ 
// | 0   4   8   12 |
// | 1   5   9   13 |
// | 2   6  10   14 |
// | 3   7  11   15 |
// _________________
//
// translation
// _________________ 
// | 0   4   8   x  |
// | 1   5   9   y  |
// | 2   6  10   z  |
// | 3   7  11   15 |
// _________________
//
// rotation x
// _________________ 
// |(1)  4   8   x  |
// | 1   xc -xs  y  |
// | 2   xs  xs  z  |
// | 3   7  11  (1) |
// _________________
//
// rotation y
// _________________ 
// | yc  4  ys   12 |
// | 1  (1)  9   13 |
// |-ys  6  yc   14 |
// | 3   7  11  (1) |
// _________________
//
// rotation z
// _________________ 
// | zc -zs  8   12 |
// | zs  zc  9   13 |
// | 2   6  (1)  14 |
// | 3   7  11  (1) |
// _________________
//
//-----------------------------------------------------------------------------

#include <math.h>
#include "mathlib.h"

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(void) {
    Identity();
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(const kexMatrix &mtx) {
    for(int i = 0; i < 4; i++) {
        vectors[i].x = mtx.vectors[i].x;
        vectors[i].y = mtx.vectors[i].y;
        vectors[i].z = mtx.vectors[i].z;
        vectors[i].w = mtx.vectors[i].w;
    }
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(const float x, const float y, const float z) {
    Identity(x, y, z);
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(const kexQuat &quat) {
    float xx = quat.x * quat.x;
    float yx = quat.y * quat.x;
    float zx = quat.z * quat.x;
    float wx = quat.w * quat.x;
    float yy = quat.y * quat.y;
    float zy = quat.z * quat.y;
    float wy = quat.w * quat.y;
    float zz = quat.z * quat.z;
    float wz = quat.w * quat.z;
    float ww = quat.w * quat.w;

    vectors[0].Set(
        ((ww + xx) - yy) - zz,
        (wz + wz) + (yx + yx),
        (zx + zx) - (wy + wy),
        0);
    vectors[1].Set(
        (yx + yx) - (wz + wz),
        (yy + (ww - xx)) - zz,
        (wx + wx) + (zy + zy),
        0);
    vectors[2].Set(
        (wy + wy + zx + zx),
        (zy + zy) - (wx + wx),
        ((ww - xx) - yy) + zz,
        0);
    vectors[3].Set(0, 0, 0, 1);
}

//
// kexMatrix::kexMatrix
//

kexMatrix::kexMatrix(const float angle, const int axis) {
    float s;
    float c;

    s = kexMath::Sin(angle);
    c = kexMath::Cos(angle);
    
    Identity();

    switch(axis) {
    case 0:
        this->vectors[0].x = c;
        this->vectors[0].z = -s;
        this->vectors[3].x = s;
        this->vectors[3].z = c;
        break;
    case 1:
        this->vectors[1].y = c;
        this->vectors[1].z = s;
        this->vectors[2].y = -s;
        this->vectors[2].z = c;
        break;
    case 2:
        this->vectors[0].x = c;
        this->vectors[0].y = s;
        this->vectors[1].x = -s;
        this->vectors[1].y = c;
        break;
    }
}

//
// kexMatrix::Identity
//

kexMatrix &kexMatrix::Identity(void) {
    vectors[0].Set(1, 0, 0, 0);
    vectors[1].Set(0, 1, 0, 0);
    vectors[2].Set(0, 0, 1, 0);
    vectors[3].Set(0, 0, 0, 1);

    return *this;
}

//
// kexMatrix::Identity
//

kexMatrix &kexMatrix::Identity(const float x, const float y, const float z) {
    vectors[0].Set(x, 0, 0, 0);
    vectors[1].Set(0, y, 0, 0);
    vectors[2].Set(0, 0, z, 0);
    vectors[3].Set(0, 0, 0, 1);

    return *this;
}

//
// kexMatrix::SetTranslation
//

kexMatrix &kexMatrix::SetTranslation(const float x, const float y, const float z) {
    vectors[3].ToVec3().Set(x, y, z);
    return *this;
}

//
// kexMatrix::SetTranslation
//

kexMatrix &kexMatrix::SetTranslation(const kexVec3 &vector) {
    vectors[3].ToVec3() = vector;
    return *this;
}

//
// kexMatrix::AddTranslation
//

kexMatrix &kexMatrix::AddTranslation(const float x, const float y, const float z) {
    vectors[3].x += x;
    vectors[3].y += y;
    vectors[3].z += z;
    return *this;
}

//
// kexMatrix::AddTranslation
//

kexMatrix &kexMatrix::AddTranslation(const kexVec3 &vector) {
    vectors[3].ToVec3() += vector;
    return *this;
}

//
// kexMatrix::Scale
//

kexMatrix &kexMatrix::Scale(const float x, const float y, const float z) {
    vectors[0].ToVec3() *= x;
    vectors[1].ToVec3() *= y;
    vectors[2].ToVec3() *= z;

    return *this;
}

//
// kexMatrix::Scale
//

kexMatrix &kexMatrix::Scale(const kexVec3 &vector) {
    vectors[0].ToVec3() *= vector.x;
    vectors[1].ToVec3() *= vector.y;
    vectors[2].ToVec3() *= vector.z;

    return *this;
}

//
// kexMatrix::Scale
//

kexMatrix kexMatrix::Scale(const kexMatrix &mtx, const float x, const float y, const float z) {
    kexMatrix out;

    out.vectors[0].ToVec3() = mtx.vectors[0].ToVec3() * x;
    out.vectors[1].ToVec3() = mtx.vectors[1].ToVec3() * y;
    out.vectors[2].ToVec3() = mtx.vectors[2].ToVec3() * z;

    return out;
}

//
// kexMatrix::Transpose
//

kexMatrix &kexMatrix::Transpose(void) {
    kexVec3 v1 = vectors[1].ToVec3();
    kexVec3 v2 = vectors[2].ToVec3();
    
    vectors[1].ToVec3() = v2;
    vectors[2].ToVec3() = v1;
    return *this;
}

//
// kexMatrix::Transpose
//

kexMatrix kexMatrix::Transpose(const kexMatrix &mtx) {
    kexMatrix out;
    
    out.vectors[0].ToVec3() = mtx.vectors[0].ToVec3();
    out.vectors[1].ToVec3() = mtx.vectors[2].ToVec3();
    out.vectors[2].ToVec3() = mtx.vectors[1].ToVec3();
    out.vectors[3].ToVec3() = mtx.vectors[3].ToVec3();

    return out;
}

//
// kexMatrix::SetViewProjection
//

void kexMatrix::SetViewProjection(float aspect, float fov, float zNear, float zFar) {
    float top       = zNear * kexMath::Tan(fov * M_PI / 360.0f);
    float bottom    = -top;
    float left      = bottom * aspect;
    float right     = top * aspect;
    
    vectors[0].x =  (2 * zNear) / (right - left);
    vectors[1].y =  (2 * zNear) / (top - bottom);
    vectors[3].z = -(2 * zFar * zNear) / (zFar - zNear);

    vectors[2].x =  (right + left) / (right - left);
    vectors[2].y =  (top + bottom) / (top - bottom);
    vectors[2].z = -(zFar + zNear) / (zFar - zNear);

    vectors[0].y = 0;
    vectors[0].z = 0;
    vectors[0].w = 0;
    vectors[1].x = 0;
    vectors[1].w = 0;
    vectors[1].z = 0;
    vectors[2].w = -1;
    vectors[3].x = 0;
    vectors[3].y = 0;
    vectors[3].w = 0;
}

//
// kexMatrix::operator*
//

kexMatrix kexMatrix::operator*(const kexVec3 &vector) {
    kexMatrix out(*this);
    
    out.vectors[3].ToVec3() +=
        vectors[0].ToVec3() * vector.x +
        vectors[1].ToVec3() * vector.y +
        vectors[2].ToVec3() * vector.z;
    return out;
}

//
// kexMatrix::operator*=
//

kexMatrix &kexMatrix::operator*=(const kexVec3 &vector) {
    vectors[3].ToVec3() +=
        vectors[0].ToVec3() * vector.x +
        vectors[1].ToVec3() * vector.y +
        vectors[2].ToVec3() * vector.z;
    return *this;
}

//
// kexMatrix::ToFloatPtr
//

float *kexMatrix::ToFloatPtr(void) {
    return reinterpret_cast<float*>(vectors);
}

//
// kexMatrix::operator*
//

kexMatrix kexMatrix::operator*(kexMatrix &matrix) {
    kexMatrix out;

    for(int i = 0; i < 4; i++) {
        out.vectors[i].x =
            vectors[i].x * matrix.vectors[0].x +
            vectors[i].y * matrix.vectors[1].x +
            vectors[i].z * matrix.vectors[2].x +
            vectors[i].w * matrix.vectors[3].x;
        out.vectors[i].y =
            vectors[i].x * matrix.vectors[0].y +
            vectors[i].y * matrix.vectors[1].y +
            vectors[i].z * matrix.vectors[2].y +
            vectors[i].w * matrix.vectors[3].y;
        out.vectors[i].z =
            vectors[i].x * matrix.vectors[0].z +
            vectors[i].y * matrix.vectors[1].z +
            vectors[i].z * matrix.vectors[2].z +
            vectors[i].w * matrix.vectors[3].z;
        out.vectors[i].w =
            vectors[i].x * matrix.vectors[0].w +
            vectors[i].y * matrix.vectors[1].w +
            vectors[i].z * matrix.vectors[2].w +
            vectors[i].w * matrix.vectors[3].w;
    }

    return out;
}

//
// kexMatrix::operator|
//

kexMatrix kexMatrix::operator|(kexMatrix &matrix) {
    kexMatrix out;

    for(int i = 0; i < 3; i++) {
        out.vectors[i].x =
            vectors[i].x * matrix.vectors[0].x +
            vectors[i].y * matrix.vectors[1].x +
            vectors[i].z * matrix.vectors[2].x;
        out.vectors[i].y =
            vectors[i].x * matrix.vectors[0].y +
            vectors[i].y * matrix.vectors[1].y +
            vectors[i].z * matrix.vectors[2].y;
        out.vectors[i].z =
            vectors[i].x * matrix.vectors[0].z +
            vectors[i].y * matrix.vectors[1].z +
            vectors[i].z * matrix.vectors[2].z;
    }

    out.vectors[3].x =
        vectors[3].x * matrix.vectors[0].x +
        vectors[3].y * matrix.vectors[1].x +
        vectors[3].z * matrix.vectors[2].x + matrix.vectors[3].x;
    out.vectors[3].y =
        vectors[3].x * matrix.vectors[0].y +
        vectors[3].y * matrix.vectors[1].y +
        vectors[3].z * matrix.vectors[2].y + matrix.vectors[3].y;
    out.vectors[3].z =
        vectors[3].x * matrix.vectors[0].z +
        vectors[3].y * matrix.vectors[1].z +
        vectors[3].z * matrix.vectors[2].z + matrix.vectors[3].z;

    return out;
}

//
// kexMatrix::operator=
//

kexMatrix &kexMatrix::operator=(const kexMatrix &matrix) {
    vectors[0] = matrix.vectors[0];
    vectors[1] = matrix.vectors[1];
    vectors[2] = matrix.vectors[2];
    vectors[3] = matrix.vectors[3];
    
    return *this;
}


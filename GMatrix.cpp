#include "include/GMatrix.h"

GMatrix::GMatrix() {
    fMat[0] = 1.0f; fMat[1] = 0.0f; fMat[2] = 0.0f;
    fMat[3] = 0.0f; fMat[4] = 1.0f; fMat[5] = 0.0f;
}

GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(1.0f, 0.0f, tx, 0.0f, 1.0f, ty);
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(sx, 0.0f, 0.0f, 0.0f, sy, 0.0f);
}

GMatrix GMatrix::Rotate(float radians) {
    float cosTheta = std::cos(radians);
    float sinTheta = std::sin(radians);
    return GMatrix(cosTheta, -sinTheta, 0.0f, sinTheta, cosTheta, 0.0f);
}

/**
 *  Return the product of two matrices: a * b
 */
GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    return GMatrix(
        a[0] * b[0] + a[1] * b[3],
        a[0] * b[1] + a[1] * b[4],
        a[0] * b[2] + a[1] * b[5] + a[2],
        a[3] * b[0] + a[4] * b[3],
        a[3] * b[1] + a[4] * b[4],
        a[3] * b[2] + a[4] * b[5] + a[5]
    );
}

/*
 *  Compute the inverse of this matrix, and store it in the "inverse" parameter, being
 *  careful to handle the case where 'inverse' might alias this matrix.
 *
 *  If this matrix is invertible, return true. If not, return false, and ignore the
 *  'inverse' parameter.
 */
bool GMatrix::invert(GMatrix* inverse) const {

    float det = fMat[0] * fMat[4] - fMat[1] * fMat[3];

    if (det == 0) {
        return false;
    }

    float invDet = 1.0f / det;

    GMatrix tmpInverse = GMatrix();

    tmpInverse.fMat[0] = fMat[4] * invDet;
    tmpInverse.fMat[1] = -fMat[1] * invDet;
    tmpInverse.fMat[2] = (fMat[1] * fMat[5] - fMat[2] * fMat[4]) * invDet;
    tmpInverse.fMat[3] = -fMat[3] * invDet;
    tmpInverse.fMat[4] = fMat[0] * invDet;
    tmpInverse.fMat[5] = (fMat[2] * fMat[3] - fMat[0] * fMat[5]) * invDet;

    *inverse = tmpInverse;

    return true;
}
    /**
     *  Transform the set of points in src, storing the resulting points in dst, by applying this
     *  matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
     *
     *  [ a  b  c ] [ x ]     x' = ax + by + c
     *  [ d  e  f ] [ y ]     y' = dx + ey + f
     *  [ 0  0  1 ] [ 1 ]
     *
     *  Note: It is legal for src and dst to point to the same memory (however, they may not
     *  partially overlap). Thus the following is supported.
     *
     *  GPoint pts[] = { ... };
     *  matrix.mapPoints(pts, pts, count);
     */
void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        float sx = src[i].x;
        float sy = src[i].y;
        dst[i].x = sx * fMat[0] + sy * fMat[1] + fMat[2];
        dst[i].y = sx * fMat[3] + sy * fMat[4] + fMat[5];
    }
}
   
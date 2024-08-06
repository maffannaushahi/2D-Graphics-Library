#include "include/GMatrix.h"

// default identity matrix
GMatrix::GMatrix() {
    fMat[0] = 1;
    fMat[1] = 0;
    fMat[2] = 0;
    fMat[3] = 1;
    fMat[4] = 0;
    fMat[5] = 0;
}

GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(1, 0, tx, 0, 1, ty);
}

GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(sx, 0, 0, 0, sy, 0);
}

GMatrix GMatrix::Rotate(float radians) {
    float cos = std::cos(radians);
    float sin = std::sin(radians);
    return GMatrix(cos, -sin, 0, sin, cos, 0);
}

GMatrix GMatrix::Concat(const GMatrix &a, const GMatrix &b) {
    return GMatrix(a[0] * b[0] + a[2] * b[1],          // a
                   a[0] * b[2] + a[2] * b[3],          // c
                   a[0] * b[4] + a[2] * b[5] + a[4],   // e
                   a[1] * b[0] + a[3] * b[1],          // b
                   a[1] * b[2] + a[3] * b[3],          // d
                   a[1] * b[4] + a[3] * b[5] + a[5]);  // f
}

// implement invert method
// invert method is supposed to redo the changes made on the matrix to get back to the original matrix

std::optional<GMatrix> GMatrix::invert() const {
    float det = fMat[0] * fMat[3] - fMat[1] * fMat[2];

    if (det == 0) {
        return {};
    }

    float inv_det = 1 / det;

    float new_a = (fMat[3] * inv_det);
    float new_b = (-fMat[1] * inv_det);
    float new_c = (-fMat[2] * inv_det);
    float new_d = (fMat[0] * inv_det);
    float new_e = (inv_det * (fMat[2] * fMat[5] - fMat[3] * fMat[4]));  // -c*f + d*e
    float new_f = (inv_det * (fMat[1] * fMat[4] - fMat[0] * fMat[5]));  // b*f - a*e

    GMatrix inv = GMatrix(new_a, new_c, new_e, new_b, new_d, new_f);

    return inv;
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        float x = fMat[0] * src[i].x + fMat[2] * src[i].y + fMat[4];
        float y = fMat[1] * src[i].x + fMat[3] * src[i].y + fMat[5];
        dst[i].x = x;
        dst[i].y = y;
    }
}

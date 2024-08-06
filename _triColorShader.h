#ifndef _TRICOLORSHADER_H_
#define _TRICOLORSHADER_H_

#include "include/GMatrix.h"
#include "include/GShader.h"
#include "iostream"

class TriColorShader : public GShader {
   public:
    TriColorShader(GPoint p0, GPoint p1, GPoint p2, GColor c0, GColor c1, GColor c2) {
        fP0 = p0;
        fP1 = p1;
        fP2 = p2;

        fC0 = c0;
        fC1 = c1;
        fC2 = c2;

        GPoint a = fP1 - fP0;
        GPoint b = fP2 - fP0;

        // GMatrix(float a, float c, float e, float b, float d, float f) {
        //  fMat[0] = a;    fMat[2] = c;    fMat[4] = e;
        //  fMat[1] = b;    fMat[3] = d;    fMat[5] = f;
        //}
        fLocalMatrix = GMatrix(a.x, b.x, fP0.x, a.y, b.y, fP0.y);
    }

    bool isOpaque() override {
        return false;
    }

    bool setContext(const GMatrix& ctm) override {
        GMatrix total_transformation = ctm * fLocalMatrix;
        std::optional<GMatrix> inverse = total_transformation.invert();
        if (!inverse.has_value()) {
            return false;
        }
        fInverse = inverse.value();
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GColor dc1 = fC1 - fC0;  // delta color 1
        GColor dc2 = fC2 - fC0;  // delta color 2
        GPoint p = GPoint{x + 0.5f, y + 0.5f};
        GPoint p_prime = fInverse * p;  // (x, y) in local space
        GColor c = p_prime.x * dc1 + p_prime.y * dc2 + fC0;
        GColor dc = fInverse[0] * dc1 + fInverse[1] * dc2;

        for (int i = 0; i < count; i++) {
            row[i] = ConvertColorToPixel(c);
            c += dc;
        }
    }

   private:
    GPoint fP0, fP1, fP2;
    GColor fC0, fC1, fC2;
    GMatrix fInverse;
    GMatrix fLocalMatrix;
};
#endif
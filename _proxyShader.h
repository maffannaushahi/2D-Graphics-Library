#ifndef _proxyShader_h
#define _proxyShader_h

#include "include/GMatrix.h"
#include "include/GShader.h"

class ProxyShader : public GShader {
   public:
    ProxyShader(GPoint p0, GPoint p1, GPoint p2, GPoint t0, GPoint t1, GPoint t2, GShader* bitmapShader) : fBitmapShader(bitmapShader) {
        fP0 = p0;
        fP1 = p1;
        fP2 = p2;

        fT0 = t0;
        fT1 = t1;
        fT2 = t2;

        GPoint a = fP1 - fP0;
        GPoint b = fP2 - fP0;

        GPoint a_t = fT1 - fT0;
        GPoint b_t = fT2 - fT0;

        textureMatrix = GMatrix(a_t.x, b_t.x, fT0.x, a_t.y, b_t.y, fT0.y);  // 2x3 matrix);
        verticeMatrix = GMatrix(a.x, b.x, fP0.x, a.y, b.y, fP0.y);          // 2x3 matrix
    }

    bool isOpaque() override {
        return fBitmapShader->isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        std::optional<GMatrix> inverseTextureMatrix = textureMatrix.invert();
        return fBitmapShader->setContext(ctm * (verticeMatrix * *inverseTextureMatrix));
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        fBitmapShader->shadeRow(x, y, count, row);
    }

   private:
    GPoint fP0, fP1, fP2, fT0, fT1, fT2;
    GShader* fBitmapShader;
    GMatrix textureMatrix, verticeMatrix;
};

#endif
#ifndef GRADIENTSHADER_H
#define GRADIENTSHADER_H

#include <vector>

#include "include/GMatrix.h"
#include "include/GShader.h"

class LinearGradientShader : public GShader {
   public:
    LinearGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode mode);
    ~LinearGradientShader();
    inline bool isOpaque() override;
    bool setContext(const GMatrix &ctm) override;
    void shadeRow(int x, int y, int count, GPixel row[]) override;

   private:
    GColor *fColors;
    GPoint fP0, fP1;
    GMatrix fInverse;
    int fCount;
    float fDeltaX, fDeltaY;
    GTileMode fMode;
};

#endif  // GRADIENTSHADER_H
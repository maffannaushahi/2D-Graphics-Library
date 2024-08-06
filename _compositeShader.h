#include "include/GMatrix.h"
#include "include/GShader.h"
#include "iostream"

class CompositeShader : public GShader {
   public:
    CompositeShader(GShader *shader1, GShader *shader2) : fShader1(shader1), fShader2(shader2) {
    }

    bool isOpaque() override {
        return fShader1->isOpaque() && fShader2->isOpaque();
    }
    bool setContext(const GMatrix &ctm) override {
        if (fShader1->setContext(ctm) && fShader2->setContext(ctm)) {
            return true;
        }
        return false;
    }
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPixel row1[count];
        GPixel row2[count];

        fShader1->shadeRow(x, y, count, row1);
        fShader2->shadeRow(x, y, count, row2);
        for (int i = 0; i < count; i++) {
            // break down row1[i] and row2[i] into their components and add them together
            // then convert the sum back to a pixel and store it in row[i]
            GPixel pixel1 = row1[i];
            GPixel pixel2 = row2[i];
            int a = div255(GPixel_GetA(pixel1) * GPixel_GetA(pixel2));
            int r = div255(GPixel_GetR(pixel1) * GPixel_GetR(pixel2));
            int g = div255(GPixel_GetG(pixel1) * GPixel_GetG(pixel2));
            int b = div255(GPixel_GetB(pixel1) * GPixel_GetB(pixel2));
            GPixel p = GPixel_PackARGB(a, r, g, b);
            row[i] = p;
        }
    }

   private:
    GShader *fShader1;
    GShader *fShader2;

    static inline unsigned div255(unsigned prod) {
        return (prod + 128) * 257 >> 16;
    }
};
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GShader.h"

class MyShader : public GShader {
   public:
    MyShader(const GBitmap &bitmap, const GMatrix &localMatrix, GTileMode mode) : fDevice(bitmap), fLocalMatrix(localMatrix), fMode(mode) {}

    inline bool isOpaque() override;
    bool setContext(const GMatrix &ctm) override;
    void shadeRow(int x, int y, int count, GPixel row[]) override;

   protected:
    GMatrix fInverse;

   private:
    GBitmap fDevice;
    const GMatrix fLocalMatrix;
    GTileMode fMode;
};

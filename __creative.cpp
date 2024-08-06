#include "_canvas.h"
#include "_clipping.h"
#include "_shader.h"

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap &bitmap) {
    if (bitmap.width() <= 0 || bitmap.height() <= 0) {
        return nullptr;
    }
    return std::unique_ptr<GCanvas>(new MyCanvas(bitmap));
}

std::string GDrawSomething(GCanvas *canvas, GISize dimension) {
    // draw a rectangle with a radial gradient

    return "Drawn!";
}

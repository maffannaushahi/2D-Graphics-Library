#include "_shader.h"

bool MyShader::isOpaque() {
    return fDevice.isOpaque();
}

bool MyShader::setContext(const GMatrix &ctm) {
    GMatrix total_transformation = ctm * fLocalMatrix;
    std::optional<GMatrix> invert_transformation = total_transformation.invert();
    if (invert_transformation.has_value()) {
        fInverse = *invert_transformation;
        return true;
    }
    return false;
}

// implement shadeRow
void MyShader::shadeRow(int x, int y, int count, GPixel row[]) {
    int bitmapWidth = fDevice.width();
    int bitmapHeight = fDevice.height();
    for (int i = 0; i < count; i++) {
        GPoint p = fInverse * GPoint{x + i + 0.5f, y + 0.5f};

        float normalizedX = p.x / bitmapWidth;
        float normalizedY = p.y / bitmapHeight;

        switch (fMode) {
            case GTileMode::kClamp:
                normalizedX = std::min(std::max(normalizedX, 0.0f), 0.999999f);
                normalizedY = std::min(std::max(normalizedY, 0.0f), 0.999999f);
                break;
            case GTileMode::kRepeat:
                normalizedX -= floorf(normalizedX);
                normalizedY -= floorf(normalizedY);
                break;
            case GTileMode::kMirror:
                normalizedX = fabsf(normalizedX - 2 * floorf(normalizedX / 2) - 1);
                normalizedY = fabsf(normalizedY - 2 * floorf(normalizedY / 2) - 1);
                normalizedX = 1 - normalizedX;  // Mirror the coordinates
                normalizedY = 1 - normalizedY;
                break;
        }

        int clampedX = std::min(std::max(0, GFloorToInt(normalizedX * bitmapWidth)), bitmapWidth - 1);
        int clampedY = std::min(std::max(0, GFloorToInt(normalizedY * bitmapHeight)), bitmapHeight - 1);

        row[i] = *fDevice.getAddr(clampedX, clampedY);
    }
}

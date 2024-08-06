#include "_blend.h"
#include "_gradientShader.h"

LinearGradientShader::LinearGradientShader(GPoint p0, GPoint p1,
                                           const GColor colors[], int count, GTileMode mode) : fMode(mode) {
    fColors = new GColor[count];
    // color array --> NOTE: IN java, it would be fine to do fColors = colors after making new array.
    // C++ things can go out of scope quick.
    for (int i = 0; i < count; i++) {
        fColors[i] = colors[i];  // copy colors
    }
    fCount = count;
    fP0 = p0;                              // starting point of gradient
    fP1 = p1;                              // ending point of gradient
    fDeltaX = fP1.x - fP0.x;               // distance between x coordinates of gradient points
    fDeltaY = fP1.y - fP0.y;               // distance between y coordinates of gradient points
    fInverse = GMatrix(1, 0, 0, 0, 1, 0);  // placeholder
}

LinearGradientShader::~LinearGradientShader() {
    delete[] fColors;  // Deallocate the color array to prevent memory leaks
}

bool LinearGradientShader::isOpaque() {
    // gradient shader is only opaque if all colors are opaque
    if (fCount == 1) {
        if (fColors[0].a != 1) {
            return false;
        }
        return true;
    } else {
        for (int i = 0; i < fCount; i++) {
            if (fColors[i].a != 1) {
                return false;
            }
        }
        return true;
    }
}

bool LinearGradientShader::setContext(const GMatrix &ctm) {
    if (fCount == 1) {
        return true;
    } else {
        std::optional<GMatrix> invertCTM = ctm.invert();
        if (!invertCTM.has_value()) {
            return false;
        }
        GMatrix deviceMatrix = GMatrix(fDeltaX, -fDeltaY, fP0.x, fDeltaY, fDeltaX, fP0.y);  // device matrix
        std::optional<GMatrix> deviceMatrixInv = deviceMatrix.invert();                     // inverse of device matrix
        if (!deviceMatrixInv.has_value()) {
            return false;
        }
        fInverse = deviceMatrixInv.value() * invertCTM.value();
        return true;
    }
}

// We now have the gradient line on the local space, so now have to interpolate the colors

void LinearGradientShader::shadeRow(int x, int y, int count, GPixel row[]) {
    if (fCount == 1) {
        float a = fColors[0].a;
        float r = fColors[0].r;
        float g = fColors[0].g;
        float b = fColors[0].b;
        for (int i = 0; i < count; i++) {
            row[i] = ConvertColorToPixel(GColor::RGBA(r, g, b, a));
        }
    } else if (fCount == 2) {
        GPoint p;
        GColor c0 = fColors[0];
        GColor c1 = fColors[1];
        float prop = 0;
        float a, r, g, b;

        p.x = x + 0.5f;
        p.y = y + 0.5f;
        p = fInverse * p;

        for (int i = 0; i < count; i++) {
            float x = p.x;

            switch (fMode) {
                case GTileMode::kClamp:
                    if (x < 0) {
                        x = 0;
                    } else if (x >= 1) {
                        x = 0.99999999999999f;
                    }
                    break;
                case GTileMode::kRepeat:
                    x = x - floor(x);
                    break;
                case GTileMode::kMirror:
                    x = x * 0.5f;
                    x = x - floorf(x);  // get the fractional part of x
                    if (x >= 0.5f) {
                        x = 1 - x;
                    }
                    x = x * 1.99999999f;
                    break;
            }
            prop = x - GFloorToInt(x);
            a = c0.a * (1 - prop) + c1.a * prop;
            r = c0.r * (1 - prop) + c1.r * prop;
            g = c0.g * (1 - prop) + c1.g * prop;
            b = c0.b * (1 - prop) + c1.b * prop;
            row[i] = ConvertColorToPixel(GColor::RGBA(r, g, b, a));
            p.x += fInverse[0];
            p.y += fInverse[3];
        }
    }

    else {
        GPoint p;
        GColor c0, c1;  // colors to be lerped
        float prop = 0;
        float a, r, g, b;  // color components

        p.x = x + 0.5f;
        p.y = y + 0.5f;
        p = fInverse * p;

        for (int i = 0; i < count; i++) {
            float x = p.x;

            switch (fMode) {
                case GTileMode::kClamp:
                    if (x < 0) {
                        x = 0;
                    } else if (x >= 1) {
                        x = 0.99999999999999f;
                    }
                    break;
                case GTileMode::kRepeat:
                    x = x - floor(x);
                    break;
                case GTileMode::kMirror:
                    x = x * 0.5f;
                    x = x - floorf(x);  // get the fractional part of x
                    if (x >= 0.5f) {
                        x = 1 - x;
                    }
                    x = x * 1.99999999f;
                    break;
            }
            prop = x * (fCount - 1);
            c0 = fColors[GFloorToInt(prop)];
            c1 = fColors[GFloorToInt(prop) + 1];

            prop = prop - GFloorToInt(prop);
            a = c0.a * (1 - prop) + c1.a * prop;
            r = c0.r * (1 - prop) + c1.r * prop;
            g = c0.g * (1 - prop) + c1.g * prop;
            b = c0.b * (1 - prop) + c1.b * prop;
            row[i] = ConvertColorToPixel(GColor::RGBA(r, g, b, a));
            p.x += fInverse[0];
            p.y += fInverse[3];
        }
    }
}

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode mode) {
    if (count < 1) {
        return nullptr;
    } else if (count == 1) {
        return std::unique_ptr<GShader>(new LinearGradientShader(p0, p1, colors, 1, mode));
    } else if (count == 2) {
        return std::unique_ptr<GShader>(new LinearGradientShader(p0, p1, colors, 2, mode));
    }
    return std::unique_ptr<GShader>(new LinearGradientShader(p0, p1, colors, count, mode));
}

#include "_blend.h"
#include "include/GFinal.h"
#include "include/GShader.h"

class LinearPositionGradient : public GShader {
   public:
    LinearPositionGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) {
        fColors = new GColor[count];
        fPos = new float[count];
        for (int i = 0; i < count; i++) {
            fColors[i] = colors[i];
            fPos[i] = pos[i];
        }
        fCount = count;
        fP0 = p0;
        fP1 = p1;
        fInverse = GMatrix();
    }

    ~LinearPositionGradient() {
        delete[] fColors;
        delete[] fPos;
    }

    bool isOpaque() override {
        for (int i = 0; i < fCount; i++) {
            if (fColors[i].a != 1) {
                return false;
            }
        }
        return true;
    }

    bool setContext(const GMatrix& ctm) override {
        GMatrix localMatrix(fP1.x - fP0.x, -(fP1.y - fP0.y), fP0.x, 0, 0, 0);
        std::optional<GMatrix> invertCTM = ctm.invert();
        if (!invertCTM.has_value()) {
            return false;
        }
        fInverse = localMatrix * invertCTM.value();
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint p;
        GColor c0, c1;  // colors to be lerped
        float prop = 0;
        float a, r, g, b;  // color components

        p.x = x + 0.5f;
        p.y = y + 0.5f;
        p = fInverse * p;

        for (int i = 0; i < count; i++) {
            float fx = p.x;
            if (fx < 0) {
                fx = 0;
            } else if (fx >= 1) {
                fx = 0.9999999999f;
            }

            for (int j = 0; j < fCount - 1; j++) {
                if (fx >= fPos[j] && fx <= fPos[j + 1]) {
                    prop = (fx - fPos[j]) / (fPos[j + 1] - fPos[j]);
                    c0 = fColors[j];
                    c1 = fColors[j + 1];
                    break;
                }
            }

            a = c0.a * (1 - prop) + c1.a * prop;
            r = c0.r * (1 - prop) + c1.r * prop;
            g = c0.g * (1 - prop) + c1.g * prop;
            b = c0.b * (1 - prop) + c1.b * prop;

            row[i] = ConvertColorToPixel(GColor::RGBA(a, r, g, b));
            p.x += fInverse[0];
        }
    }

   private:
    GColor* fColors;
    int fCount;
    GPoint fP0;
    GPoint fP1;
    GMatrix fInverse;
    float* fPos;
};

class VoronoiShader : public GShader {
   public:
    VoronoiShader(const GPoint points[], const GColor colors[], int count) {
        fPoints = new GPoint[count];
        fColors = new GColor[count];
        for (int i = 0; i < count; i++) {
            fPoints[i] = points[i];
            fColors[i] = colors[i];
        }
        fCount = count;
    }

    ~VoronoiShader() {
        delete[] fPoints;
        delete[] fColors;
    }

    bool isOpaque() override {
        for (int i = 0; i < fCount; i++) {
            if (fColors[i].a != 1) {
                return false;
            }
        }
        return true;
    }

    bool setContext(const GMatrix& ctm) override {
        std::optional<GMatrix> invertCTM = ctm.invert();
        if (!invertCTM.has_value()) {
            return false;
        }
        fInverse = invertCTM.value();
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        for (int i = 0; i < count; i++) {
            GPoint minDist = {x + 0.5f + i, y + 0.5f};
            GPoint p = fInverse * minDist;
            float closestDist = std::numeric_limits<float>::max();
            int index;
            for (int j = 0; j < fCount; j++) {
                float dist = sqrt(pow(fPoints[j].x - p.x, 2) + pow(fPoints[j].y - p.y, 2));
                if (dist < closestDist) {
                    closestDist = dist;
                    index = j;
                }
            }
            row[i] = ConvertColorToPixel(fColors[index]);
        }
    }

   private:
    GPoint* fPoints;
    GColor* fColors;
    int fCount;
    GMatrix fInverse;
};

class Final : public GFinal {
   public:
    std::unique_ptr<GShader> createLinearPosGradient(GPoint p0, GPoint p1,
                                                     const GColor colors[],
                                                     const float pos[],
                                                     int count) override {
        return std::make_unique<LinearPositionGradient>(p0, p1, colors, pos, count);
    }

    std::unique_ptr<GShader> createVoronoiShader(const GPoint points[],
                                                 const GColor colors[],
                                                 int count) override {
        return std::make_unique<VoronoiShader>(points, colors, count);
    }

    virtual GPath strokePolygon(const GPoint points[], int count, float width, bool isClosed) override {
        GPath path;
        GPoint p0;
        GPoint p1;
        GPoint v;
        GPoint vPerp;
        GPoint p0Offset;
        float vPerpLength;
        float radius = width / 2;

        for (int i = 0; i < count - 1; i++) {
            p0 = points[i];
            p1 = points[i + 1];

            v = p1 - p0;
            vPerp = GPoint{-1 * v.y, v.x};

            vPerpLength = sqrt(vPerp.x * vPerp.x + vPerp.y * vPerp.y);

            p0Offset = {vPerp.x * radius / vPerpLength, vPerp.y * radius / vPerpLength};
            GPoint ptsConnect[4] = {p0 + p0Offset, p1 + p0Offset, p1 - p0Offset, p0 - p0Offset};

            path.addPolygon(ptsConnect, 4);
            path.addCircle(p0, radius, GPath::kCCW_Direction);
            path.addCircle(p1, radius, GPath::kCCW_Direction);
        }

        if (isClosed) {
            p0 = points[count - 1];
            p1 = points[0];

            v = p1 - p0;
            vPerp = GPoint{-1 * v.y, v.x};

            vPerpLength = sqrt(vPerp.x * vPerp.x + vPerp.y * vPerp.y);

            p0Offset = {vPerp.x * radius / vPerpLength, vPerp.y * radius / vPerpLength};
            GPoint ptsConnect[4] = {p0 + p0Offset, p1 + p0Offset, p1 - p0Offset, p0 - p0Offset};

            path.addPolygon(ptsConnect, 4);
            path.addCircle(p0, radius, GPath::kCCW_Direction);
            path.addCircle(p1, radius, GPath::kCCW_Direction);
        }
        return path;
    }
};

std::unique_ptr<GFinal> GCreateFinal() {
    return std::unique_ptr<GFinal>(new Final());
}
#include <iostream>

#include "_curves.h"
#include "include/GPath.h"

void GPath::addRect(const GRect& r, Direction dir) {
    GPoint p1 = {r.left, r.top};
    GPoint p2 = {r.right, r.top};
    GPoint p3 = {r.right, r.bottom};
    GPoint p4 = {r.left, r.bottom};

    // direction is clockwise by default
    if (dir == Direction::kCW_Direction) {
        this->moveTo(p1);
        this->lineTo(p2);
        this->lineTo(p3);
        this->lineTo(p4);
    } else {
        this->moveTo(p1);
        this->lineTo(p4);
        this->lineTo(p3);
        this->lineTo(p2);
    }
}

void GPath::addPolygon(const GPoint pts[], int count) {
    for (int i = 0; i < count; i++) {
        if (i == 0) {
            this->moveTo(pts[i]);
        } else {
            this->lineTo(pts[i]);
        }
    }
}

void GPath::addCircle(GPoint center, float radius, Direction dir) {
    GPoint p0, p1;
    float x = center.x;
    float y = center.y;

    float unitCircleX[17] = {1, 1, std::sqrt(2) / 2, tan(M_PI / 8), 0, -1 * tan(M_PI / 8), -1 * std::sqrt(2) / 2, -1, -1, -1, -1 * std::sqrt(2) / 2, -1 * tan(M_PI / 8), 0, tan(M_PI / 8), std::sqrt(2) / 2, 1, 1};
    float unitCircleY[17] = {0, -1 * tan(M_PI / 8), -1 * std::sqrt(2) / 2, -1, -1, -1, -1 * std::sqrt(2) / 2, -1 * tan(M_PI / 8), 0, tan(M_PI / 8), std::sqrt(2) / 2, 1, 1, 1, std::sqrt(2) / 2, tan(M_PI / 8), 0};

    p0 = {x + radius, y};

    this->moveTo(p0);

    if (dir == kCCW_Direction) {
        for (int i = 1; i < 16; i += 2) {
            p0 = (GPoint){x + (radius * unitCircleX[i]), y + radius * unitCircleY[i]};
            p1 = (GPoint){x + (radius * unitCircleX[i + 1]), y + radius * unitCircleY[i + 1]};
            this->quadTo(p0, p1);
        }
    } else {
        for (int i = 15; i > 0; i -= 2) {
            p0 = (GPoint){x + (radius * unitCircleX[i]), y + radius * unitCircleY[i]};
            p1 = (GPoint){x + (radius * unitCircleX[i - 1]), y + radius * unitCircleY[i - 1]};
            this->quadTo(p0, p1);
        }
    }
}
void GPath::transform(const GMatrix& m) {
    int numfPts = countPoints();
    GPoint* dst = new GPoint[numfPts];
    m.mapPoints(dst, fPts.data(), numfPts);

    // clear the fPts vector to repopulate it with the transformed points in dst
    fPts.clear();
    for (int i = 0; i < numfPts; i++) {
        fPts.push_back(dst[i]);
    }

    delete[] dst;  // cleanup
}

GRect GPath::bounds() const {
    if (fPts.empty()) return GRect::WH(0, 0);

    // Instantiate minX, minY, maxX, maxY to the first point
    float minX = fPts[0].x;
    float minY = fPts[0].y;
    float maxX = fPts[0].x;
    float maxY = fPts[0].y;

    GPath path = *this;
    GPath::Edger edger(path);
    GPoint pts[GPath::kMaxNextPoints];
    std::optional<GPath::Verb> verbOpt;

    while ((verbOpt = edger.next(pts)).has_value()) {
        GPath::Verb verb = *verbOpt;
        switch (verb) {
            case GPath::Verb::kLine:
                minX = std::min(minX, pts[1].x);
                minY = std::min(minY, pts[1].y);
                maxX = std::max(maxX, pts[1].x);
                maxY = std::max(maxY, pts[1].y);
                break;

            case GPath::Verb::kQuad: {
                GPoint p0 = pts[0];
                GPoint p1 = pts[1];  // control pt
                GPoint p2 = pts[2];

                float ax = p0.x - 2 * p1.x + p2.x;
                float ay = p0.y - 2 * p1.y + p2.y;
                float bx = 2 * (p1.x - p0.x);
                float by = 2 * (p1.y - p0.y);

                if (ax == 0 && ay == 0) {
                    // The curve is linear; just use the end points.
                    minX = std::min({minX, p0.x, p2.x});
                    minY = std::min({minY, p0.y, p2.y});
                    maxX = std::max({maxX, p0.x, p2.x});
                    maxY = std::max({maxY, p0.y, p2.y});
                    continue;  // Skip the rest of the loop iteration
                }

                float denominatorX = 2 * ax;

                if (denominatorX == 0) {
                    continue;
                }

                float tx = -bx / (2 * ax);

                float denominatorY = 2 * ay;
                if (denominatorY == 0) {
                    continue;
                }
                float ty = -by / (2 * ay);

                float stationaryX = (1 - tx) * (1 - tx) * p0.x + 2 * tx * (1 - tx) * p1.x + tx * tx * p2.x;
                float stationaryY = (1 - ty) * (1 - ty) * p0.y + 2 * ty * (1 - ty) * p1.y + ty * ty * p2.y;

                minX = std::min(minX, std::min(stationaryX, std::min(p0.x, p2.x)));
                maxX = std::max(maxX, std::max(stationaryX, std::max(p0.x, p2.x)));
                minY = std::min(minY, std::min(stationaryY, std::min(p0.y, p2.y)));
                maxY = std::max(maxY, std::max(stationaryY, std::max(p0.y, p2.y)));

                // std::cout << "Bounds after quad: (" << minX << ", " << minY << "), (" << maxX << ", " << maxY << ")\n";
                break;
            }

            case GPath::Verb::kCubic: {
                GPoint p0 = pts[0];
                GPoint p1 = pts[1];
                GPoint p2 = pts[2];
                GPoint p3 = pts[3];

                minX = std::min(minX, std::min(p0.x, p3.x));
                minY = std::min(minY, std::min(p0.y, p3.y));
                maxX = std::max(maxX, std::max(p0.x, p3.x));
                maxY = std::max(maxY, std::max(p0.y, p3.y));

                // Corrected cubic Bezier coefficients using p0, p1, p2, p3
                float ax = -3 * p0.x + 9 * p1.x - 9 * p2.x + 3 * p3.x;
                float bx = 6 * p0.x - 12 * p1.x + 6 * p2.x;
                float cx = -3 * p0.x + 3 * p1.x;

                float ay = -3 * p0.y + 9 * p1.y - 9 * p2.y + 3 * p3.y;
                float by = 6 * p0.y - 12 * p1.y + 6 * p2.y;
                float cy = -3 * p0.y + 3 * p1.y;

                // if (fabs(ax) > epsilon || fabs(bx) > epsilon || fabs(cx) > epsilon) {
                std::vector<float> rootsX = findRoots(ax, bx, cx);
                for (const auto& t : rootsX) {
                    if (t > 0 && t < 1) {  // Clamping t
                        float stationaryX = (1 - t) * (1 - t) * (1 - t) * p0.x +
                                            3 * t * (1 - t) * (1 - t) * p1.x +
                                            3 * t * t * (1 - t) * p2.x +
                                            t * t * t * p3.x;
                        ;  // Bezier equation for X
                        minX = std::min(minX, stationaryX);
                        maxX = std::max(maxX, stationaryX);
                    }
                }
                //}
                // if (fabs(ay) > epsilon || fabs(by) > epsilon || fabs(cy) > epsilon) {
                std::vector<float> rootsY = findRoots(ay, by, cy);
                for (const auto& t : rootsY) {
                    if (t > 0 && t < 1) {  // Clamping t
                        float stationaryY = (1 - t) * (1 - t) * (1 - t) * p0.y +
                                            3 * t * (1 - t) * (1 - t) * p1.y +
                                            3 * t * t * (1 - t) * p2.y +
                                            t * t * t * p3.y;  // Bezier equation for Y
                        minY = std::min(minY, stationaryY);
                        maxY = std::max(maxY, stationaryY);
                    }
                }

                // std::cout << "Bounds after cubic: (" << minX << ", " << minY << "), (" << maxX << ", " << maxY << ")\n";
                break;
            }

            default:
                break;
        }
    }
    // std::cout << "Final bounds from bounds() function: (" << minX << ", " << minY << "), (" << maxX << ", " << maxY << ")\n";

    return GRect::LTRB(minX, minY, maxX, maxY);
}

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    GPoint ab = lerp(src[0], src[1], t);
    GPoint bc = lerp(src[1], src[2], t);
    GPoint abc = lerp(ab, bc, t);
    dst[0] = src[0];  // start point
    dst[1] = ab;
    dst[2] = abc;
    dst[3] = bc;
    dst[4] = src[2];  // end point
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    GPoint ab = lerp(src[0], src[1], t);
    GPoint bc = lerp(src[1], src[2], t);
    GPoint cd = lerp(src[2], src[3], t);
    GPoint abc = lerp(ab, bc, t);
    GPoint bcd = lerp(bc, cd, t);
    GPoint abcd = lerp(abc, bcd, t);
    dst[0] = src[0];  // start point
    dst[1] = ab;
    dst[2] = abc;
    dst[3] = abcd;  // mid point (shared)
    dst[4] = bcd;
    dst[5] = cd;
    dst[6] = src[3];  // end point
}
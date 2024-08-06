#include <iostream>

#include "_curves.h"

std::vector<float> findRoots(float a, float b, float c, float d) {
    std::vector<float> roots;
    float discriminant = b * b - 4 * a * c;
    if (a == 0) {
        if (b == 0) {
            return roots;
        }
        float t = -c / b;
        roots.push_back(t);
    } else {
        if (discriminant < 0) {
            return roots;
        }
        if (discriminant == 0) {
            roots.push_back(-b / (2 * a));
        } else {
            float sqrtDiscriminant = std::sqrt(discriminant);
            roots.push_back((-b + sqrtDiscriminant) / (2 * a));
            roots.push_back((-b - sqrtDiscriminant) / (2 * a));
        }
    }

    return roots;
}

GPoint lerp(const GPoint &a, const GPoint &b, float t) {
    return GPoint{(1 - t) * a.x + t * b.x, (1 - t) * a.y + t * b.y};
}

GPoint bilerp(GPoint a, GPoint b, GPoint c, GPoint d, float u, float v) {
    float computedX = (1 - u) * (1 - v) * a.x + u * (1 - v) * b.x + u * v * c.x + (1 - u) * v * d.x;
    float computedY = (1 - u) * (1 - v) * a.y + u * (1 - v) * b.y + u * v * c.y + (1 - u) * v * d.y;
    return GPoint{computedX, computedY};
}

GColor bilerp(GColor a, GColor b, GColor c, GColor d, float u, float v) {
    float computedR = (1 - u) * (1 - v) * a.r + u * (1 - v) * b.r + u * v * c.r + (1 - u) * v * d.r;
    float computedG = (1 - u) * (1 - v) * a.g + u * (1 - v) * b.g + u * v * c.g + (1 - u) * v * d.g;
    float computedB = (1 - u) * (1 - v) * a.b + u * (1 - v) * b.b + u * v * c.b + (1 - u) * v * d.b;
    float computedA = (1 - u) * (1 - v) * a.a + u * (1 - v) * b.a + u * v * c.a + (1 - u) * v * d.a;
    return GColor{computedR, computedG, computedB, computedA};
}

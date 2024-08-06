#include <iostream>

#include "include/GBitmap.h"
#include "include/GMath.h"
#include "include/GPath.h"
#include "include/GPoint.h"
#include "include/GRect.h"

struct Edge {
    float m;
    float b;
    int top;
    int bottom;
    int winding_val;
    float currentX;

    Edge(const GPoint& p1, const GPoint& p2, int winding) {
        if (p1.y < p2.y) {
            top = GRoundToInt(p1.y);
            bottom = GRoundToInt(p2.y);
        } else {
            top = GRoundToInt(p2.y);
            bottom = GRoundToInt(p1.y);
            winding *= -1;  // Reverse winding if p2 is above p1
        }

        if (p1.y != p2.y) {
            m = (p2.x - p1.x) / (p2.y - p1.y);
            b = p1.x - m * p1.y;
        }
        winding_val = winding;
        currentX = (top + 0.5) * m + b;
    }

    // is Edge active?
    bool isValid(float y) const {
        return y >= top && y < bottom;
    }

    // compute x value for given y
    float computeX(float y) const {
        return m * y + b;
    }
};

std::vector<Edge> createEdges(const GPoint vertices[], int count, int top, int bottom);

bool verticalClipping(GPoint& p1, GPoint& p2, int top, int bottom);

std::vector<GPoint> horizontalClipping(GPoint& p1, GPoint& p2, int left, int right);

std::vector<Edge> createPathEdges(GPoint p0, GPoint p1, int bottom, int right);

std::vector<Edge> processPath(const GPath& path, const GBitmap& fDevice);

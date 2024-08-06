#include "_clipping.h"

bool verticalClipping(GPoint &p1, GPoint &p2, int top, int bottom) {
    float m = (p2.x - p1.x) / (p2.y - p1.y);

    if (p2.y < top || p1.y > bottom) {
        // The whole edge is out of bounds; discard it
        return false;
    }

    if (p1.y < top) {
        p1.x = p1.x + m * (top - p1.y);
        p1.y = top;
    }

    if (p2.y > bottom) {
        p2.x = p2.x + m * (bottom - p2.y);
        p2.y = bottom;
    }

    return true;
}

std::vector<GPoint> horizontalClipping(GPoint &p1, GPoint &p2, int left, int right) {
    if (p1.x > p2.x) {
        std::swap(p1, p2);
    }

    float m = (p2.y - p1.y) / (p2.x - p1.x);

    if (p2.x < left) {
        // project -> compute new p1 and p2
        // p2.y and p1.y is same, the x value changes to 0 (start of canvas)
        p1.x = left;
        p2.x = left;
        return {p1, p2};
    }

    if (p1.x > right) {
        p1.x = right;  // change the x to max width
        p2.x = right;
        return {p1, p2};
    }

    std::vector<GPoint> pts;

    if (p1.x < left) {
        // bend -> compute + project
        //  find intersection using similar triangles
        GPoint bendLeft = {left, p1.y};
        pts.push_back(bendLeft);
        p1.y += m * (left - p1.x);
        p1.x = left;
    }

    if (p2.x > right) {
        // bend -> compute + project
        GPoint bendRight = {right, p2.y};
        pts.push_back(bendRight);
        p2.y += m * (right - p2.x);
        p2.x = right;
    }

    pts.push_back(p1);
    pts.push_back(p2);
    return pts;
}

std::vector<Edge> createEdges(const GPoint vertices[], int count, int maxHeight, int maxWidth) {
    std::vector<Edge> edges;
    for (int i = 0; i < count; i++) {
        GPoint p1 = vertices[i];
        GPoint p2 = vertices[(i + 1) % count];

        if (GRoundToInt(p1.y) == GRoundToInt(p2.y))
            continue;

        if (p1.y > p2.y) {
            std::swap(p1, p2);
        }

        if (!verticalClipping(p1, p2, 0, maxHeight))
            continue;

        auto clippedPts = horizontalClipping(p1, p2, 0, maxWidth);

        std::sort(clippedPts.begin(), clippedPts.end(), [](const GPoint &a, const GPoint &b) { return a.y > b.y; });

        for (size_t j = 0; j + 1 < clippedPts.size(); j++) {
            edges.push_back(Edge(clippedPts[j], clippedPts[j + 1], 1));
        }
    }
    return edges;
}

std::vector<Edge> createPathEdges(GPoint p0, GPoint p1, int bottom, int right) {
    std::vector<Edge> windedEdges;
    int winding = 1;

    if (p0.y > p1.y) {
        std::swap(p0, p1);
        winding = -1;
    }

    // Apply vertical clipping
    if (!verticalClipping(p0, p1, 0, bottom)) {
        // Edge is completely out of vertical bounds
        return windedEdges;
    }

    // Apply horizontal clipping
    auto clippedPts = horizontalClipping(p0, p1, 0, right);

    std::sort(clippedPts.begin(), clippedPts.end(), [](const GPoint &a, const GPoint &b) { return a.y > b.y; });

    // In case horizontal clipping introduces bends, create edges for each segment
    for (size_t i = 0; i + 1 < clippedPts.size(); ++i) {
        GPoint start = clippedPts[i];
        GPoint end = clippedPts[i + 1];
        Edge e = Edge(start, end, winding);

        // Check for horizontal edges, which we can ignore in non-zero winding rule
        if (e.top != e.bottom) {
            windedEdges.push_back(e);
        }
    }

    return windedEdges;
}

std::vector<Edge> processPath(const GPath &path, const GBitmap &fDevice) {
    std::vector<Edge> edges;
    // GRect bounds = GRect::WH(fDevice.width(), fDevice.height());

    GPath::Edger edger(path);
    GPoint pts[GPath::kMaxNextPoints];
    std::optional<GPath::Verb> verbOpt;
    float tolerance = 0.25f;

    while ((verbOpt = edger.next(pts))) {
        GPath::Verb verb = *verbOpt;  // dereference the optional

        if (verb == GPath::kLine) {
            auto pathEdges = createPathEdges(pts[0], pts[1], fDevice.height(), fDevice.width());
            edges.insert(edges.end(), pathEdges.begin(), pathEdges.end());
        }

        if (verb == GPath::kQuad) {
            GPoint error = 0.25 * (pts[0] - (2 * pts[1]) + pts[2]);
            int num_segs = GCeilToInt(std::sqrt(error.length() / tolerance));
            float dt = 1.0f / num_segs;

            GPoint clippedPt1 = pts[0];
            GPoint clippedPt2 = pts[1];

            for (float t = dt; t < 1; t += dt) {
                GPoint newPt = ((1 - t) * (1 - t) * pts[0]) + (2 * t * (1 - t) * pts[1]) + (t * t * pts[2]);
                clippedPt2 = newPt;
                auto pathEdges = createPathEdges(clippedPt1, clippedPt2, fDevice.height(), fDevice.width());
                edges.insert(edges.end(), pathEdges.begin(), pathEdges.end());
                clippedPt1 = clippedPt2;
            }

            clippedPt2 = pts[2];  // last point
            auto pathEdges = createPathEdges(clippedPt1, clippedPt2, fDevice.height(), fDevice.width());
            edges.insert(edges.end(), pathEdges.begin(), pathEdges.end());
        }

        if (verb == GPath::kCubic) {
            GPoint e1 = pts[0] - (2 * pts[1]) + pts[2];                                       // error
            GPoint e2 = pts[1] - (2 * pts[2]) + pts[3];                                       // error
            GPoint error = {std::max(abs(e1.x), abs(e2.x)), std::max(abs(e1.y), abs(e2.y))};  // max error

            int num_segs = GCeilToInt(std::sqrt(3 * error.length()));
            float dt = 1.0f / num_segs;

            GPoint clippedPt1 = pts[0];
            GPoint clippedPt2 = pts[1];

            for (float t = dt; t < 1; t += dt) {
                GPoint newPt = (1 - t) * (1 - t) * (1 - t) * pts[0] + 3 * t * (1 - t) * (1 - t) * pts[1] + 3 * t * t * (1 - t) * pts[2] + t * t * t * pts[3];
                clippedPt2 = newPt;
                auto pathEdges = createPathEdges(clippedPt1, clippedPt2, fDevice.height(), fDevice.width());
                edges.insert(edges.end(), pathEdges.begin(), pathEdges.end());
                clippedPt1 = clippedPt2;
            }
            clippedPt2 = pts[3];  // last point
            auto pathEdges = createPathEdges(clippedPt1, clippedPt2, fDevice.height(), fDevice.width());
            edges.insert(edges.end(), pathEdges.begin(), pathEdges.end());
        }
    }
    return edges;
}

//
// Created by Affan Naushahi on 1/21/24.
//
#include "_blend.h"
#include "_canvas.h"
#include "_clipping.h"
#include "_compositeShader.h"
#include "_curves.h"
#include "_proxyShader.h"
#include "_shader.h"
#include "_triColorShader.h"

void MyCanvas::save() {
    ctmStack.push(ctmStack.top());
}

// restore the current CTM (pop the top of the stack and set it as the current CTM)
void MyCanvas::restore() {
    ctmStack.pop();
}

// new top of stack = top of stack * matrix
void MyCanvas::concat(const GMatrix &matrix) {
    GMatrix topMatrix = ctmStack.top();
    topMatrix = topMatrix * matrix;
    ctmStack.pop();
    ctmStack.push(topMatrix);
}

void MyCanvas::clear(const GColor &color) {
    GPixel pixel = ConvertColorToPixel(color);

    for (int y = 0; y < fDevice.height(); y++) {
        GPixel *addr = fDevice.getAddr(0, y);
        std::fill(addr, addr + fDevice.width(), pixel);
    }
}

template <typename Func>
void drawRectTemplate(Func blendFunc, const GRect &rect, const GPaint &paint, GBitmap fDevice) {  // making sure that rectangle is not out of bounds. CLAMPING
    GIRect giRect = rect.round();
    giRect.left = std::max(0, giRect.left);
    giRect.top = std::max(0, giRect.top);
    giRect.right = std::min(fDevice.width(), giRect.right);
    giRect.bottom = std::min(fDevice.height(), giRect.bottom);

    if (giRect.left >= fDevice.width() || giRect.top >= fDevice.height() || giRect.right <= 0 || giRect.bottom <= 0) {
        return;
    }

    GPixel srcPixel = ConvertColorToPixel(paint.getColor());
    GShader *shader = paint.getShader();

    int count = giRect.width();

    if (shader) {
        if (blendFunc == kSrc) {
            GPixel rowPixels[count];
            for (int y = giRect.top; y < giRect.bottom; y++) {
                shader->shadeRow(giRect.left, y, count, rowPixels);
                for (int x = giRect.left; x < giRect.right; x++) {
                    GPixel *addr = fDevice.getAddr(x, y);
                    *addr = rowPixels[x - giRect.left];
                }
            }
        }

        else if (blendFunc == kClear) {
            for (int y = giRect.top; y < giRect.bottom; y++) {
                GPixel *addr = fDevice.getAddr(giRect.left, y);
                std::fill(addr, addr + count, 0);
            }
        }

        else {
            GPixel rowPixels[count];
            for (int y = giRect.top; y < giRect.bottom; y++) {
                shader->shadeRow(giRect.left, y, count, rowPixels);
                for (int x = giRect.left; x < giRect.right; x++) {
                    GPixel *addr = fDevice.getAddr(x, y);
                    *addr = blendFunc(*addr, rowPixels[x - giRect.left]);
                }
            }
        }
    }

    else {
        if (blendFunc == kSrc) {
            for (int y = giRect.top; y < giRect.bottom; y++) {
                GPixel *addr = fDevice.getAddr(giRect.left, y);
                std::fill(addr, addr + count, srcPixel);
            }
        } else if (blendFunc == kClear) {
            for (int y = giRect.top; y < giRect.bottom; y++) {
                GPixel *addr = fDevice.getAddr(giRect.left, y);
                std::fill(addr, addr + count, 0);
            }
        } else {
            for (int y = giRect.top; y < giRect.bottom; y++) {
                GPixel prevAddr = 1;
                GPixel prevBlend;
                for (int x = giRect.left; x < giRect.right; x++) {
                    GPixel *addr = fDevice.getAddr(x, y);
                    if (*addr == prevAddr) {
                        *addr = prevBlend;
                        continue;
                    }
                    prevAddr = *addr;
                    prevBlend = blendFunc(*addr, srcPixel);
                    *addr = prevBlend;
                }
            }
        }
    }
}

void MyCanvas::drawRect(const GRect &rect, const GPaint &paint) {
    GMatrix ctm = ctmStack.top();
    GPoint vertices[] = {
        {rect.left, rect.top},
        {rect.right, rect.top},
        {rect.right, rect.bottom},
        {rect.left, rect.bottom}};

    // if the ctm is only rotational, pass it to convex polygon
    if (ctm[1] != 0 || ctm[2] != 0) {
        drawConvexPolygon(vertices, 4, paint);
        return;
    }

    ctm.mapPoints(vertices, vertices, 4);
    GRect newRect = GRect::LTRB(vertices[0].x, vertices[0].y, vertices[2].x, vertices[2].y);

    if (paint.getShader() && !paint.getShader()->setContext(ctm)) {
        return;
    }

    GBlendMode blendmode;
    if (paint.getShader()) {
        blendmode = paint.getShader()->isOpaque() ? optimize_mode(paint.getBlendMode(), ConvertColorToPixel(paint.getColor())) : paint.getBlendMode();
    } else {
        blendmode = optimize_mode(paint.getBlendMode(), ConvertColorToPixel(paint.getColor()));
    }

    if (blendmode == GBlendMode::kDst) {
        return;
    }

    switch (blendmode) {
        case GBlendMode::kClear:
            drawRectTemplate(kClear, newRect, paint, fDevice);
            break;
        case GBlendMode::kSrc:
            drawRectTemplate(kSrc, newRect, paint, fDevice);
            break;
        case GBlendMode::kDst:
            drawRectTemplate(kDst, newRect, paint, fDevice);
            break;
        case GBlendMode::kSrcOver:
            drawRectTemplate(kSrcOver, newRect, paint, fDevice);
            break;
        case GBlendMode::kSrcIn:
            drawRectTemplate(kSrcIn, newRect, paint, fDevice);
            break;
        case GBlendMode::kDstOver:
            drawRectTemplate(kDstOver, newRect, paint, fDevice);
            break;
        case GBlendMode::kDstIn:
            drawRectTemplate(kDstIn, newRect, paint, fDevice);
            break;
        case GBlendMode::kSrcOut:
            drawRectTemplate(kSrcOut, newRect, paint, fDevice);
            break;
        case GBlendMode::kDstOut:
            drawRectTemplate(kDstOut, newRect, paint, fDevice);
            break;
        case GBlendMode::kSrcATop:
            drawRectTemplate(kSrcATop, newRect, paint, fDevice);
            break;
        case GBlendMode::kDstATop:
            drawRectTemplate(kDstATop, newRect, paint, fDevice);
            break;
        case GBlendMode::kXor:
            drawRectTemplate(kXor, newRect, paint, fDevice);
            break;
        default:
            drawRectTemplate(kClear, newRect, paint, fDevice);
            break;
    }
}

template <typename Func>
void drawConvexPolygonTemplate(Func blendFunc, const GPoint vertices[], int count, const GPaint &paint, const GBitmap fDevice) {
    // bounds of the canvas
    int canvasHeight = fDevice.height() - 1;
    int canvasWidth = fDevice.width() - 1;

    // create clipped edges at start
    std::vector<Edge> edges = createEdges(vertices, count, canvasHeight, canvasWidth);

    if (edges.size() < 2) {
        return;
    }

    std::sort(edges.begin(), edges.end(), [](const Edge &a, const Edge &b) {
        if (a.top != b.top) return a.top < b.top;
        return a.bottom < b.bottom; });

    GShader *shader = paint.getShader();
    GPixel srcPixel = ConvertColorToPixel(paint.getColor());

    int y = edges[0].top;
    int fpointer = 0;
    int spointer = 1;
    int temp = 2;

    while (temp <= edges.size()) {
        Edge &edge1 = edges[fpointer];
        Edge &edge2 = edges[spointer];
        float center = y + 0.5;

        // if edge1 is out of bounds update the pointers
        if (center > edge1.bottom) {
            fpointer = temp;
            temp++;
            continue;
        }
        // if edge2 is out of bounds update the pointers
        if (center > edge2.bottom) {
            spointer = temp;
            temp++;
            continue;
        }

        // calculate the x intercepts

        float xIntercept1 = edge1.m * center + edge1.b;
        float xIntercept2 = edge2.m * center + edge2.b;

        if (xIntercept1 > xIntercept2) {
            std::swap(xIntercept1, xIntercept2);
        }

        int startX = GRoundToInt(xIntercept1);
        int endX = GRoundToInt(xIntercept2);
        int span = endX - startX;
        if (span != 0) {
            if (shader) {
                // new row pixels
                GPixel rowPixels[span];
                shader->shadeRow(startX, y, span, rowPixels);

                for (int x = startX; x < endX; x++) {
                    GPixel *addr = fDevice.getAddr(x, y);
                    *addr = blendFunc(*addr, rowPixels[x - startX]);
                }
            } else {
                if (blendFunc == kSrc) {
                    GPixel *addr = fDevice.getAddr(startX, y);
                    std::fill(addr, addr + (span), srcPixel);
                }

                else if (blendFunc == kClear) {
                    GPixel *addr = fDevice.getAddr(startX, y);
                    std::fill(addr, addr + (span), 0);
                }

                else {
                    GPixel prevAddr = 1;
                    GPixel prevBlend;
                    for (int x = startX; x < endX; x++) {
                        GPixel *addr = fDevice.getAddr(x, y);
                        if (*addr == prevAddr) {
                            *addr = prevBlend;
                            continue;
                        }
                        prevAddr = *addr;
                        prevBlend = blendFunc(*addr, srcPixel);
                        *addr = prevBlend;
                    }
                }
            }
        }
        y++;
    }
}

void MyCanvas::drawConvexPolygon(const GPoint vertices[], int count, const GPaint &paint) {
    if (count < 3)
        return;

    GMatrix ctm = ctmStack.top();
    GPoint transformedVertices[count];
    ctm.mapPoints(transformedVertices, vertices, count);

    if (paint.getShader() && !paint.getShader()->setContext(ctm)) {
        return;
    }

    GBlendMode blendmode;
    if (paint.getShader()) {
        blendmode = paint.getShader()->isOpaque() ? optimize_mode(paint.getBlendMode(), ConvertColorToPixel(paint.getColor())) : paint.getBlendMode();
    } else {
        blendmode = optimize_mode(paint.getBlendMode(), ConvertColorToPixel(paint.getColor()));
    }

    if (blendmode == GBlendMode::kDst) {
        return;
    }
    switch (blendmode) {
        case GBlendMode::kClear:
            drawConvexPolygonTemplate(kClear, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kSrc:
            drawConvexPolygonTemplate(kSrc, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kDst:
            drawConvexPolygonTemplate(kDst, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kSrcOver:
            drawConvexPolygonTemplate(kSrcOver, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kSrcIn:
            drawConvexPolygonTemplate(kSrcIn, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kDstOver:
            drawConvexPolygonTemplate(kDstOver, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kDstIn:
            drawConvexPolygonTemplate(kDstIn, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kSrcOut:
            drawConvexPolygonTemplate(kSrcOut, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kDstOut:
            drawConvexPolygonTemplate(kDstOut, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kSrcATop:
            drawConvexPolygonTemplate(kSrcATop, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kDstATop:
            drawConvexPolygonTemplate(kDstATop, transformedVertices, count, paint, fDevice);
            break;
        case GBlendMode::kXor:
            drawConvexPolygonTemplate(kXor, transformedVertices, count, paint, fDevice);
            break;
        default:
            drawConvexPolygonTemplate(kSrcOver, transformedVertices, count, paint, fDevice);
            break;
    }
}

template <typename Func>
void drawPathTemplate(Func blendFunc, const GPath &path, int count, const GPaint &paint, const GBitmap &fDevice) {
    // GRect bounds = path.bounds();
    // GIRect roundedBounds = bounds.round();

    GRect bounds;
    bounds.left = 0;
    bounds.top = 0;
    bounds.right = fDevice.width();
    bounds.bottom = fDevice.height();
    GIRect roundedBounds = bounds.round();

    // make sure edges are clipped and processed correctly.
    std::vector<Edge> pathEdges = processPath(path, fDevice);

    if (pathEdges.size() < 2) {
        return;
    }

    std::sort(pathEdges.begin(), pathEdges.end(), [](Edge edge1, Edge edge2) {
        if (edge1.top == edge2.top) {
            return edge1.m * (edge1.top + .5) + edge1.b < edge2.m * (edge2.top + .5) + edge2.b;
        }
        return edge1.top < edge2.top;
    });

    GShader *shader = paint.getShader();
    GPixel srcPixel = ConvertColorToPixel(paint.getColor());

    for (int y = roundedBounds.top; y < roundedBounds.bottom; y++) {
        size_t i = 0;
        int w = 0;
        int L = 0;
        float center = y + 0.5;

        while (i < pathEdges.size() && pathEdges[i].isValid(center)) {
            int x = GRoundToInt(pathEdges[i].computeX(center));
            if (w == 0) {
                L = x;
            }
            w += pathEdges[i].winding_val;
            if (w == 0) {
                int R = x;

                // if (R < L) {
                //     std::swap(L, R);
                // }
                assert(R >= L);
                int span = R - L;
                if (span != 0) {
                    if (shader) {
                        GPixel rowPixels[R - L];
                        shader->shadeRow(L, y, R - L, rowPixels);
                        for (int x = L; x < R; x++) {
                            GPixel *addr = fDevice.getAddr(x, y);
                            *addr = blendFunc(*addr, rowPixels[x - L]);
                        }
                    } else {
                        if (blendFunc == kSrc) {
                            GPixel *addr = fDevice.getAddr(L, y);
                            std::fill(addr, addr + (R - L), srcPixel);
                        } else if (blendFunc == kClear) {
                            GPixel *addr = fDevice.getAddr(L, y);
                            std::fill(addr, addr + (R - L), 0);
                        }

                        else {
                            GPixel *addr = fDevice.getAddr(L, y);
                            for (int x = L; x < R; x++) {
                                *addr = blendFunc(*addr, srcPixel);
                                addr++;
                            }
                        }
                    }
                }
            }
            if (pathEdges[i].isValid(center + 1)) {
                // pathEdges[i].currentX += pathEdges[i].m;
                i++;
            } else {
                pathEdges.erase(pathEdges.begin() + i);
            }
        }

        assert(w == 0);

        while (i < pathEdges.size() && pathEdges[i].isValid(center + 1)) {
            i += 1;
        }

        std::sort(pathEdges.begin(), pathEdges.begin() + i, [center](const Edge &a, const Edge &b) {
            return a.computeX(center + 1) < b.computeX(center + 1);
        });
    }
}

void MyCanvas::drawPath(const GPath &path, const GPaint &paint) {
    GPath transformedPath = path;
    int count = transformedPath.countPoints();
    transformedPath.transform(ctmStack.top());

    if (paint.getShader() && !paint.getShader()->setContext(ctmStack.top())) {
        return;
    }

    GBlendMode blendmode;
    if (paint.getShader()) {
        blendmode = paint.getShader()->isOpaque() ? optimize_mode(paint.getBlendMode(), ConvertColorToPixel(paint.getColor())) : paint.getBlendMode();
    } else {
        blendmode = optimize_mode(paint.getBlendMode(), ConvertColorToPixel(paint.getColor()));
    }

    if (blendmode == GBlendMode::kDst) {
        return;
    }

    switch (blendmode) {
        case GBlendMode::kClear:
            drawPathTemplate(kClear, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kSrc:
            drawPathTemplate(kSrc, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kDst:
            drawPathTemplate(kDst, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kSrcOver:
            drawPathTemplate(kSrcOver, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kSrcIn:
            drawPathTemplate(kSrcIn, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kDstOver:
            drawPathTemplate(kDstOver, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kDstIn:
            drawPathTemplate(kDstIn, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kSrcOut:
            drawPathTemplate(kSrcOut, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kDstOut:
            drawPathTemplate(kDstOut, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kSrcATop:
            drawPathTemplate(kSrcATop, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kDstATop:
            drawPathTemplate(kDstATop, transformedPath, count, paint, fDevice);
            break;
        case GBlendMode::kXor:
            drawPathTemplate(kXor, transformedPath, count, paint, fDevice);
            break;
        default:
            drawPathTemplate(kSrcOver, transformedPath, count, paint, fDevice);
            break;
    }
}

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                        int count, const int indices[], const GPaint &paint) {
    GPoint p0, p1, p2, t0, t1, t2;
    GColor c0, c1, c2;

    int n = 0;

    for (int i = 0; i < count; i++) {
        int i0 = indices[n + 0];
        int i1 = indices[n + 1];
        int i2 = indices[n + 2];

        p0 = verts[i0];
        p1 = verts[i1];
        p2 = verts[i2];

        GPoint triVertices[3] = {p0, p1, p2};

        if (colors) {
            c0 = colors[i0];
            c1 = colors[i1];
            c2 = colors[i2];
            TriColorShader *shaderT = new TriColorShader(p0, p1, p2, c0, c1, c2);

            if (texs && paint.getShader() != nullptr) {
                t0 = texs[i0];
                t1 = texs[i1];
                t2 = texs[i2];
                ProxyShader *shaderP = new ProxyShader(p0, p1, p2, t0, t1, t2, paint.getShader());
                CompositeShader *shaderC = new CompositeShader(shaderT, shaderP);
                GPaint paintC = GPaint(shaderC);
                drawConvexPolygon(triVertices, 3, paintC);
            } else {
                GPaint paintT = GPaint(shaderT);
                drawConvexPolygon(triVertices, 3, paintT);
            }

        } else if (paint.getShader() != nullptr && texs) {
            // bitmapshader -- proxyshader
            t0 = texs[i0];
            t1 = texs[i1];
            t2 = texs[i2];

            // implement ProxyShader
            ProxyShader *shaderP = new ProxyShader(p0, p1, p2, t0, t1, t2, paint.getShader());
            GPaint paintP = GPaint(shaderP);
            drawConvexPolygon(triVertices, 3, paintP);
        }
        n += 3;
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                        int level, const GPaint &paint) {
    GPoint myVerts[4];
    GColor myColors[4];
    GPoint myTexs[4];

    int n = level + 1;

    int indices[6] = {0, 1, 2, 1, 3, 2};

    for (int y = 0; y < n; y++) {
        for (int x = 0; x < n; x++) {
            float u = x / float(n);
            float v = y / float(n);

            myVerts[0] = bilerp(verts[0], verts[1], verts[2], verts[3], u, v);
            myVerts[1] = bilerp(verts[0], verts[1], verts[2], verts[3], u + 1.0f / n, v);
            myVerts[2] = bilerp(verts[0], verts[1], verts[2], verts[3], u, v + 1.0f / n);
            myVerts[3] = bilerp(verts[0], verts[1], verts[2], verts[3], u + 1.0f / n, v + 1.0f / n);

            GColor *cp = nullptr;
            GPoint *tp = nullptr;

            if (texs) {
                myTexs[0] = bilerp(texs[0], texs[1], texs[2], texs[3], u, v);
                myTexs[1] = bilerp(texs[0], texs[1], texs[2], texs[3], u + 1.0f / n, v);
                myTexs[2] = bilerp(texs[0], texs[1], texs[2], texs[3], u, v + 1.0f / n);
                myTexs[3] = bilerp(texs[0], texs[1], texs[2], texs[3], u + 1.0f / n, v + 1.0f / n);

                tp = myTexs;
            }
            if (colors) {
                myColors[0] = bilerp(colors[0], colors[1], colors[2], colors[3], u, v);
                myColors[1] = bilerp(colors[0], colors[1], colors[2], colors[3], u + 1.0f / n, v);
                myColors[2] = bilerp(colors[0], colors[1], colors[2], colors[3], u, v + 1.0f / n);
                myColors[3] = bilerp(colors[0], colors[1], colors[2], colors[3], u + 1.0f / n, v + 1.0f / n);
                cp = myColors;
            }
            drawMesh(myVerts, cp, tp, 2, indices, paint);
        }
    }
}

// save the current CTM (push a copy onto the stack)
std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap &bitmap, const GMatrix &matrix, GTileMode mode) {
    std::optional<GMatrix> invMatrix = matrix.invert();
    if (invMatrix.has_value()) {
        return std::unique_ptr<GShader>(new MyShader(bitmap, matrix, mode));
    } else {
        return nullptr;
    }
}

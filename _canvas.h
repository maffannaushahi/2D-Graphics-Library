//
// Created by Affan Naushahi on 1/21/24.
//

#ifndef PA1_MAFFANNAUSHAHI_MAIN_MYCANVAS_H
#define PA1_MAFFANNAUSHAHI_MAIN_MYCANVAS_H

#include <stack>

#include "include/GBitmap.h"
#include "include/GCanvas.h"
#include "include/GColor.h"
#include "include/GPaint.h"
#include "include/GRect.h"

class MyCanvas : public GCanvas {
   public:
    MyCanvas(const GBitmap &device) : fDevice(device) {
        ctmStack.push(GMatrix());
    }

    virtual void clear(const GColor &color) override;

    virtual void drawRect(const GRect &rect, const GPaint &color) override;

    virtual void drawConvexPolygon(const GPoint vertices[], int count, const GPaint &color) override;

    virtual void save() override;

    virtual void restore() override;

    virtual void concat(const GMatrix &matrix) override;

    virtual void drawPath(const GPath &path, const GPaint &paint) override;

    virtual void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint &) override;

    virtual void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                          int level, const GPaint &) override;

   private:
    const GBitmap fDevice;
    std::stack<GMatrix> ctmStack;
};

#endif  // PA1_MAFFANNAUSHAHI_MAIN_MYCANVAS_H

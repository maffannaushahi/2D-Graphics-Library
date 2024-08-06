#include <iostream>
#include <map>

#include "include/GCanvas.h"
#include "include/GMath.h"
#include "include/GPixel.h"

// INLINE BLENDING.

inline GPixel ConvertColorToPixel(const GColor& color) {
    unsigned a = GRoundToInt(color.a * 255);
    unsigned r = GRoundToInt(color.r * color.a * 255);
    unsigned g = GRoundToInt(color.g * color.a * 255);
    unsigned b = GRoundToInt(color.b * color.a * 255);

    return GPixel_PackARGB(a, r, g, b);
}

inline GBlendMode optimize_mode(GBlendMode requested, GPixel pixel) {
    unsigned alpha = GPixel_GetA(pixel);

    if (alpha == 0) {
        std::map<GBlendMode, GBlendMode> noAlpha = {
            {GBlendMode::kClear, GBlendMode::kClear},
            {GBlendMode::kDst, GBlendMode::kDst},
            {GBlendMode::kSrc, GBlendMode::kClear},
            {GBlendMode::kSrcOver, GBlendMode::kDst},
            {GBlendMode::kDstOver, GBlendMode::kDst},
            {GBlendMode::kSrcIn, GBlendMode::kClear},
            {GBlendMode::kSrcOut, GBlendMode::kClear},
            {GBlendMode::kDstIn, GBlendMode::kClear},
            {GBlendMode::kDstOut, GBlendMode::kDst},
            {GBlendMode::kSrcATop, GBlendMode::kDst},
            {GBlendMode::kDstATop, GBlendMode::kClear},
            {GBlendMode::kXor, GBlendMode::kDst}};
        return noAlpha[requested];
    }

    if (alpha == 255) {
        std::map<GBlendMode, GBlendMode> fullAlpha = {
            {GBlendMode::kClear, GBlendMode::kClear},
            {GBlendMode::kDst, GBlendMode::kDst},
            {GBlendMode::kSrc, GBlendMode::kSrc},
            {GBlendMode::kSrcOver, GBlendMode::kSrc},
            {GBlendMode::kDstOver, GBlendMode::kDstOver},
            {GBlendMode::kSrcIn, GBlendMode::kSrcIn},
            {GBlendMode::kSrcOut, GBlendMode::kSrcOut},
            {GBlendMode::kDstIn, GBlendMode::kDst},
            {GBlendMode::kDstOut, GBlendMode::kClear},
            {GBlendMode::kSrcATop, GBlendMode::kSrcIn},
            {GBlendMode::kDstATop, GBlendMode::kDstOver},
            {GBlendMode::kXor, GBlendMode::kSrcOut}};
        return fullAlpha[requested];
    }

    // in case no optimization return what came in
    return requested;
}

static inline unsigned division(unsigned product) {
    return (product + 128) * 257 >> 16;
}

inline GPixel kClear(GPixel dest, GPixel src) {
    return 0;
}

inline GPixel kDst(GPixel dest, GPixel src) {
    return dest;
}

inline GPixel kSrc(GPixel dest, GPixel src) {
    return src;
}

inline GPixel kDstOver(GPixel dest, GPixel src) {
    int dstAlpha = GPixel_GetA(dest);
    int invAlpha = 255 - dstAlpha;

    unsigned outR = GPixel_GetR(dest) + division(invAlpha * GPixel_GetR(src));
    unsigned outG = GPixel_GetG(dest) + division(invAlpha * GPixel_GetG(src));
    unsigned outB = GPixel_GetB(dest) + division(invAlpha * GPixel_GetB(src));
    unsigned outA = GPixel_GetA(dest) + division(invAlpha * GPixel_GetA(src));

    return GPixel_PackARGB(outA, outR, outG, outB);
}

inline GPixel kSrcOver(GPixel dest, GPixel src) {
    int srcAlpha = GPixel_GetA(src);
    int invAlpha = 255 - srcAlpha;

    unsigned outR = GPixel_GetR(src) + division(invAlpha * GPixel_GetR(dest));
    unsigned outG = GPixel_GetG(src) + division(invAlpha * GPixel_GetG(dest));
    unsigned outB = GPixel_GetB(src) + division(invAlpha * GPixel_GetB(dest));
    unsigned outA = GPixel_GetA(src) + division(invAlpha * GPixel_GetA(dest));

    return GPixel_PackARGB(outA, outR, outG, outB);
}

inline GPixel kSrcIn(GPixel dest, GPixel src) {
    int dstAlpha = GPixel_GetA(dest);

    unsigned outR = division(dstAlpha * GPixel_GetR(src));
    unsigned outG = division(dstAlpha * GPixel_GetG(src));
    unsigned outB = division(dstAlpha * GPixel_GetB(src));
    unsigned outA = division(dstAlpha * GPixel_GetA(src));

    return GPixel_PackARGB(outA, outR, outG, outB);
}

inline GPixel kSrcOut(GPixel dest, GPixel src) {
    int dstAlpha = GPixel_GetA(dest);
    int invAlpha = 255 - dstAlpha;

    unsigned outR = division(invAlpha * GPixel_GetR(src));
    unsigned outG = division(invAlpha * GPixel_GetG(src));
    unsigned outB = division(invAlpha * GPixel_GetB(src));
    unsigned outA = division(invAlpha * GPixel_GetA(src));

    return GPixel_PackARGB(outA, outR, outG, outB);
}

inline GPixel kDstIn(GPixel dest, GPixel src) {
    int srcAlpha = GPixel_GetA(src);

    unsigned outR = division(srcAlpha * GPixel_GetR(dest));
    unsigned outG = division(srcAlpha * GPixel_GetG(dest));
    unsigned outB = division(srcAlpha * GPixel_GetB(dest));
    unsigned outA = division(srcAlpha * GPixel_GetA(dest));

    return GPixel_PackARGB(outA, outR, outG, outB);
}

inline GPixel kDstOut(GPixel dest, GPixel src) {
    int srcAlpha = GPixel_GetA(src);
    int invAlpha = 255 - srcAlpha;

    unsigned outR = division(invAlpha * GPixel_GetR(dest));
    unsigned outG = division(invAlpha * GPixel_GetG(dest));
    unsigned outB = division(invAlpha * GPixel_GetB(dest));
    unsigned outA = division(invAlpha * GPixel_GetA(dest));

    return GPixel_PackARGB(outA, outR, outG, outB);
}

inline GPixel kSrcATop(GPixel dest, GPixel src) {
    int dstAlpha = GPixel_GetA(dest);
    int invAlpha = 255 - GPixel_GetA(src);

    unsigned outR = division(invAlpha * GPixel_GetR(dest) + dstAlpha * GPixel_GetR(src));
    unsigned outG = division(invAlpha * GPixel_GetG(dest) + dstAlpha * GPixel_GetG(src));
    unsigned outB = division(invAlpha * GPixel_GetB(dest) + dstAlpha * GPixel_GetB(src));
    unsigned outA = division(invAlpha * GPixel_GetA(dest) + dstAlpha * GPixel_GetA(src));

    return GPixel_PackARGB(outA, outR, outG, outB);
}

inline GPixel kDstATop(GPixel dest, GPixel src) {
    int srcAlpha = GPixel_GetA(src);
    int invAlpha = 255 - GPixel_GetA(dest);

    unsigned outR = division(invAlpha * GPixel_GetR(src) + srcAlpha * GPixel_GetR(dest));
    unsigned outG = division(invAlpha * GPixel_GetG(src) + srcAlpha * GPixel_GetG(dest));
    unsigned outB = division(invAlpha * GPixel_GetB(src) + srcAlpha * GPixel_GetB(dest));
    unsigned outA = division(invAlpha * GPixel_GetA(src) + srcAlpha * GPixel_GetA(dest));

    return GPixel_PackARGB(outA, outR, outG, outB);
}

inline GPixel kXor(GPixel dest, GPixel src) {
    int invSrcAlpha = 255 - GPixel_GetA(src);
    int invDstAlpha = 255 - GPixel_GetA(dest);

    unsigned outR = division(invSrcAlpha * GPixel_GetR(dest) + invDstAlpha * GPixel_GetR(src));
    unsigned outG = division(invSrcAlpha * GPixel_GetG(dest) + invDstAlpha * GPixel_GetG(src));
    unsigned outB = division(invSrcAlpha * GPixel_GetB(dest) + invDstAlpha * GPixel_GetB(src));
    unsigned outA = division(invSrcAlpha * GPixel_GetA(dest) + invDstAlpha * GPixel_GetA(src));

    return GPixel_PackARGB(outA, outR, outG, outB);
}
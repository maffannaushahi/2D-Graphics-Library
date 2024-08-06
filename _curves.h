
#include "include/GColor.h"
#include "include/GPoint.h"

std::vector<float> findRoots(float a, float b, float c, float d = 0);
GPoint lerp(const GPoint &a, const GPoint &b, float t);
GPoint bilerp(GPoint a, GPoint b, GPoint c, GPoint d, float u, float v);
GColor bilerp(GColor a, GColor b, GColor c, GColor d, float u, float v);
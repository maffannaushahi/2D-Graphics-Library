/**
 *  Copyright 2020 Mike Reed
 */

#include "image_final.cpp"

const GDrawRec gDrawRecs[] = {
    { final_sweep, 512, 512, "final_sweep", 7 },
    { final_coons, 512, 512, "final_coons", 7 },
    { final_colormarix, 512, 512, "final_colormatrix", 7 },
    { final_stroke, 512, 512, "final_stroke", 7 },
    { final_voronoi, 512, 512, "final_voronoi", 7 },
    { final_linearpos, 512, 512, "final_linearpos", 7 },

    { nullptr, 0, 0, nullptr },
};

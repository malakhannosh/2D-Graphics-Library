#include<iostream>
#include<vector>

#include "include/GCanvas.h"
#include "include/GBitmap.h"
#include "CanvasHelpers.h"

// convert helper function
int32_t convert(const GColor& c) { 

        int newA = GRoundToInt(c.a * 255);
        int newR = GRoundToInt(c.r * c.a * 255);
        int newG = GRoundToInt(c.g * c.a * 255);
        int newB = GRoundToInt(c.b * c.a * 255);

        int pixel = GPixel_PackARGB(newA,newR,newG,newB);

        return pixel;
    };

Edge makeEdge(GPoint p0, GPoint p1){
    Edge edge;
    edge.m = (p1.x - p0.x) / (p1.y - p0.y);
    edge.b = p0.x - edge.m * p0.y;
    edge.top = (p0.y < p1.y) ? GRoundToInt(p0.y) : GRoundToInt(p1.y);
    edge.bot = (p0.y > p1.y) ? GRoundToInt(p0.y) : GRoundToInt(p1.y);
    edge.wind = (p0.y < p1.y) ? 1 : -1;
    return edge;
}

void sortEdges(std::vector<Edge>& edges) {
    std::sort(edges.begin(), edges.end(), [](const Edge& e1, const Edge& e2) {
        if (e1.top == e2.top){
            return e1.calcX(e1.top) < e2.calcX(e2.top);
        }
        return e1.top < e2.top;
    });
}

std::vector<Edge> clip(const GRect& rect, std::vector<Edge> edges){
    std::vector<Edge> clips;
    for (Edge edge : edges) {
        if (edge.bot <= rect.top && edge.top >= rect.bottom){
            continue;
        }
        if (edge.top < rect.top){
            edge.top = 0;
        }
        if (edge.bot > rect.bottom){
            edge.bot = GRoundToInt(rect.bottom);
        }
        if (edge.top < edge.bot){
            clips.push_back(edge);
        }
    }
    return clips;
}

GPixel modulate(GPixel a, GPixel b){
    int A = GPixel_GetA(a) * GPixel_GetA(b) / 255;
    int R = GPixel_GetR(a) * GPixel_GetR(b) / 255;
    int G = GPixel_GetG(a) * GPixel_GetG(b) / 255;
    int B = GPixel_GetB(a) * GPixel_GetB(b) / 255;
    return GPixel_PackARGB(A,R,G,B);
}
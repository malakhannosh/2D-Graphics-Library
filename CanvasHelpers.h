#include "include/GCanvas.h"
#include "include/GBitmap.h"

// convert helper function
int32_t convert(const GColor& c);

// edge struct
struct Edge {
    float m, b;
    int top, bot;
    int wind;
    float calcX(int row) const {
        return m * (row + 0.5f) + b;
    }
    bool isValid(int y) {
        if (y < top || y >= bot){return false;}
        else {return true;}
    }
};

Edge makeEdge(GPoint p0, GPoint p1);

void sortEdges(std::vector<Edge>& edges);

std::vector<Edge> clip(const GRect& rect, std::vector<Edge> edges);

GPixel modulate(GPixel a, GPixel b);
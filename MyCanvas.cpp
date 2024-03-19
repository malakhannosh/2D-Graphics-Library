#include<iostream>
#include<vector>
#include<stack>

#include "include/GCanvas.h"
#include "include/GBitmap.h"
#include "include/GShader.h"
#include "BlendMath.h"
#include "CanvasHelpers.h"
#include "include/GPath.h"

std::unique_ptr<GShader> GCreateTriGradient(GColor c0, GColor c1, GColor c2, GMatrix baryM);
std::unique_ptr<GShader> GCreateProxy(GShader* shader, const GMatrix& extraTransform);
std::unique_ptr<GShader> GComposeShader(GShader* s0, GShader* s1);

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {
        GMatrix identityMatrix;
        matrices.push(identityMatrix);
    }

    void save(){
        matrices.push(matrices.top());
    }

    void restore(){
        if (matrices.size() > 1) {
            matrices.pop();
        }
    }

    void concat(const GMatrix& matrix) override {
        matrices.top() = GMatrix::Concat(matrices.top(), matrix);
    }

    void clear(const GColor& color) override {
        GPixel* pixels = fDevice.pixels();
        int width = fDevice.width();
        int height = fDevice.height();
        size_t rowBytes = fDevice.rowBytes();

        GPixel src = convert(color);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                GPixel* pixelAddr = pixels + x + (y * (rowBytes >> 2));
                *pixelAddr = src;
            }
        }
    }

    void drawRect(const GRect& rect, const GPaint& paint) override {
        GPoint poly[4];
        poly[0] = {rect.left, rect.top};
        poly[1] = {rect.right, rect.top};
        poly[2] = {rect.right, rect.bottom};
        poly[3] = {rect.left, rect.bottom};

        drawConvexPolygon(poly, 4, paint);
    }

    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override {
        
        int width = fDevice.width();
        int height = fDevice.height();

        GBlendMode mode = paint.getBlendMode();
        GShader *shader = paint.getShader();
        if (shader) {
            shader->setContext(matrices.top());
        }

        GPixel src = convert(paint.getColor());

        std::vector<GPoint> transformedPoints(count);
        for (int i = 0; i < count; ++i) {
            transformedPoints[i] = matrices.top() * points[i];
        }

        std::vector<Edge> edges;

        for (int i = 0; i < count; ++i) {
            Edge edge = makeEdge(transformedPoints[i], transformedPoints[(i + 1) % count]);
            if (edge.top != edge.bot){
                edges.push_back(edge);
            }
        }
        
        std::vector<Edge> clips = clip(GRect::LTRB(0,0,width,height), edges);
        sortEdges(clips);

        for (int row = 0; row < height; row++) {
            std::vector<Edge> actives;

            for (const Edge& edge : clips) {
                if (row >= edge.top && row < edge.bot){
                    actives.push_back(edge);
                }
            }

            std::sort(actives.begin(), actives.end(), [&](const Edge& e0, const Edge& e1){
                return (e0.calcX(row)) < (e1.calcX(row));
            });

            for (size_t i = 0; i < actives.size(); i += 2) {
                int x0 = std::max(GRoundToInt(actives[i].calcX(row)), 0);
                x0 = std::min(x0, width);
                int x1 = std::min(GRoundToInt(actives[i+1].calcX(row)), width);
                x1 = std::max(x1, 0);

                if (shader){
                    blit(x0, row, x1-x0, shader, mode);
                }
                else {
                    blit(x0, row, x1-x0, src, mode);
                }                
            }
        }
    }

    void drawPath(const GPath& path, const GPaint& paint) override {
        if (path.countPoints() == 0) {
            return;
        }

        int width = fDevice.width();
        int height = fDevice.height();

        GBlendMode mode = paint.getBlendMode();
        GShader* shader = paint.getShader();
        if (shader) {
            shader->setContext(matrices.top());
        }

        GColor color = paint.getColor();
        GPixel src = convert(color);

        GPoint pts[GPath::kMaxNextPoints];
        GPath::Edger edger(path);
        GPath::Verb v;

        std::vector<Edge> edges;
        GMatrix ctm = matrices.top();

        while ((v = edger.next(pts)) != GPath::kDone) {
            switch (v) {
                case GPath::kLine:
                    {
                    GPoint p0 = ctm*pts[0];
                    GPoint p1 = ctm*pts[1];
                    edges.push_back(makeEdge(p0,p1));           
                    break;
                    }

                case GPath::kQuad:
                    {
                    GPoint qp0 = ctm*pts[0];
                    GPoint qp1 = ctm*pts[1];
                    GPoint qp2 = ctm*pts[2];

                    int numSegments = 24;
                    GPoint A;
                    GPoint B;
                    
                    for (int i = 0; i < numSegments; i++){
                        float tA = (float)i/numSegments;
                        float tB = (float)(i+1)/numSegments;
                        A = calcQuad(qp0,qp1,qp2,tA);
                        B = calcQuad(qp0,qp1,qp2,tB);
                        edges.push_back(makeEdge(A,B));
                    }
                    break;
                    }
                
                case GPath::kCubic:
                    {
                    GPoint cp0 = ctm*pts[0];
                    GPoint cp1 = ctm*pts[1];
                    GPoint cp2 = ctm*pts[2];
                    GPoint cp3 = ctm*pts[3];

                    
                    int numSegments = 24;
                    GPoint A;
                    GPoint B;

                    for (int i = 0; i < numSegments; i++){
                        float tA = (float)i/numSegments;
                        float tB = (float)(i+1)/numSegments;
                        A = calcCubic(cp0,cp1,cp2,cp3,tA);
                        B = calcCubic(cp0,cp1,cp2,cp3,tB);
                        edges.push_back(makeEdge(A,B));
                    }
                    break;
                    }
            }
        }
        
        std::vector<Edge> clipped = clip(GRect::LTRB(0,0,width,height), edges);
        // printf("clipped size: %d \n", clipped.size());
        if (clipped.size() < 2){
            return;
        }
        sortEdges(clipped);
        int ymax = clipped[0].bot;

        for (int i = 1; i < clipped.size(); i++){
            ymax = std::max(ymax, clipped[i].bot);
        }

        for (int y = clipped[0].top; y < ymax; ++y) {
            int i = 0;
            int w = 0;
            int L;

            while (i < clipped.size() && clipped[i].isValid(y)) {
                int x = std::min(GRoundToInt(clipped[i].calcX(y)), width);
                x = std::max(x, 0);
    
                if (w == 0) {
                    L = x;
                }
                w += clipped[i].wind;
                if (w == 0) {
                    int R = x;
                    if (shader){
                        blit(L,y,R-L,shader,mode);
                    }
                    else{
                        blit(L,y,R-L,src,mode);
                    }
                }
                if (clipped[i].isValid(y+1)){
                    i++;
                }
                else {
                    assert(i < clipped.size());
                    clipped.erase(clipped.begin() + i);
                }
            }
            assert(w==0);
            while(i < clipped.size() && clipped[i].isValid(y+1)){
                i++;
            }

            std::sort(clipped.begin(), clipped.begin() + i,
                    [y](const Edge a, const Edge b) {
                        return a.calcX(y + 1) < b.calcX(y + 1);
                    });
        }
    }

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) {
        if (count < 1){
            return;
        }

        int n = 0;
        for(int i = 0; i < count; i++){
            GPoint pts[3];
            pts[0] = verts[indices[n]];
            pts[1] = verts[indices[n+1]];
            pts[2] = verts[indices[n+2]];

            GMatrix pM = {pts[1].x-pts[0].x, pts[2].x-pts[0].x, pts[0].x, pts[1].y-pts[0].y, pts[2].y-pts[0].y, pts[0].y};

            if(colors && !texs){
                GColor c0 = colors[indices[n]];
                GColor c1 = colors[indices[n+1]];
                GColor c2 = colors[indices[n+2]];

                GPaint triPaint;
                auto cShader = GCreateTriGradient(c0,c1,c2,pM);
                triPaint.setShader(cShader.get());

                drawConvexPolygon(pts, 3, triPaint);
            }

            if(paint.getShader()){
                GShader* shader = paint.getShader();
                if(texs && !colors){
                    GPoint t0 = texs[indices[n]];
                    GPoint t1 = texs[indices[n+1]];
                    GPoint t2 = texs[indices[n+2]];

                    GMatrix tM = {t1.x-t0.x, t2.x-t0.x, t0.x, t1.y-t0.y, t2.y-t0.y, t0.y};
                    GMatrix invTM;
                    tM.invert(&invTM);
                    GMatrix p = pM * invTM;

                    GPaint texPaint;
                    auto tShader = GCreateProxy(shader, p);
                    texPaint.setShader(tShader.get());

                    drawConvexPolygon(pts, 3, texPaint);
                }
                if(texs && colors){
                    GColor c0 = colors[indices[n]];
                    GColor c1 = colors[indices[n+1]];
                    GColor c2 = colors[indices[n+2]];

                    GPoint t0 = texs[indices[n]];
                    GPoint t1 = texs[indices[n+1]];
                    GPoint t2 = texs[indices[n+2]];

                    GMatrix tM = {t1.x-t0.x, t2.x-t0.x, t0.x, t1.y-t0.y, t2.y-t0.y, t0.y};
                    GMatrix invTM;
                    tM.invert(&invTM);
                    GMatrix p = pM * invTM;

                    auto cShader = GCreateTriGradient(c0,c1,c2,pM);
                    auto tShader = GCreateProxy(shader, p);
                    
                    GPaint compPaint;
                    auto compShader = GComposeShader(cShader.get(), tShader.get());
                    compPaint.setShader(compShader.get());

                    drawConvexPolygon(pts,3,compPaint);
                }
            }       
            n += 3;
        }
    }

    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint){
        if (level < 0){
            return;
        }
        
        const int size = GRoundToInt((level + 2) * (level + 2));
        GPoint vrt[size];
        GPoint tex[size];
        GColor col[size];

        const int count = GRoundToInt((level + 1) * (level + 1) * 2);
        int ind[count * 3];

        int lindex = 0;
        int i = 0;
        int row = 0;

        for (int u = 0; u < level + 2; u++){
            lindex = u;
            for (int v = 0; v < level + 2; v++){
                row = (level + 2);
                lindex = u + GRoundToInt(row * v);

                if (u < level + 1 && v < level + 1){
                    ind[i] = lindex;
                    ind[i + 1] = lindex + 1;
                    ind[i + 2] = lindex + row;
                    ind[i + 3] = lindex + row;
                    ind[i + 4] = lindex + row + 1;
                    ind[i + 5] = lindex + 1;
                    i += 6;
                }

                float divU = u/(float)(level+1);
                float divV = v/(float)(level+1);

                vrt[lindex] = bilerp(divU,divV,verts[0],verts[1],verts[2],verts[3]);

                if (texs){
                    tex[lindex] = bilerp(divU,divV,texs[0],texs[1],texs[2],texs[3]);
                }

                if (colors){
                    col[lindex] = bilerp(divU,divV,colors[0],colors[1],colors[2],colors[3]);
                }
            }
        }
        drawMesh(vrt, colors ? col: nullptr, texs ? tex: nullptr, count, ind, paint);
    }

    GPoint lerp(GPoint a, GPoint b, float x){
        return a + (x*(b-a));
    }

    GColor lerp(GColor a, GColor b, float x){
        return a + (x*(b-a));
    }

    GPoint bilerp(float u, float v, GPoint a, GPoint b, GPoint c, GPoint d){
        // (1-v)(1-u)A + (1-v)uB + v(1-u)D + vuC
        return lerp(lerp(a,b,u),lerp(d,c,u),v);
    }

    GColor bilerp(float u, float v, GColor a, GColor b, GColor c, GColor d){
        return lerp(lerp(a,b,u),lerp(d,c,u),v);
    }

    void blit(int x, int y, int count, GPixel src, GBlendMode mode){
        for (int i = 0; i < count; i++){
            GPixel* dst = fDevice.getAddr(x+i, y);
            *dst = blend(mode, src, *dst);
        }
    }

    void blit(int x, int y, int count, GShader* shader, GBlendMode mode){
        std::vector<GPixel> storage(count);
        shader->shadeRow(x, y, count, &storage[0]);
        for (int i = 0; i < count; i++){
            GPixel* dst = fDevice.getAddr(x+i, y);
            *dst = blend(mode, storage[i], *dst);
        }
    }

    GPoint calcQuad(GPoint A, GPoint B, GPoint C, float t){
        // (1-t)^2A + 2t(1-t)B + t^2C
        return (1-t)*(1-t)*A + 2*t*(1-t)*B + t*t*C;
    }


    GPoint calcCubic(GPoint A, GPoint B, GPoint C, GPoint D, float t){
        // (1-t)^3A + 3t(1-t)^2B + 3t^2(1-t)C + t^3D;
        return (1-t)*(1-t)*(1-t)*A + 3*t*(1-t)*(1-t)*B + 3*t*t*(1-t)*C + t*t*t*D;
    }

    float mag(GPoint E){
        return sqrt(E.x*E.x + E.y*E.y);
    }

private:
    const GBitmap fDevice;
    std::stack<GMatrix> matrices;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    GColor color = GColor::RGBA(10,10,5,0);
    canvas->clear(color);
    
    std::vector<GPoint> polygon = {
        {5,10},
        {0,2},
        {10,4}
    };

    GPaint paint;
    paint.setColor(GColor::RGBA(1.0f, 0.0f, 0.0f, 1.0f));

    canvas->drawConvexPolygon(polygon.data(), polygon.size(), paint);

    return "some red";
}
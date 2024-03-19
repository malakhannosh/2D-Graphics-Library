#include "include/GPath.h"


void GPath::addRect(const GRect& rect, Direction direction){
    moveTo({rect.left, rect.top});
    if (direction == kCW_Direction){
        lineTo({rect.right, rect.top});
        lineTo({rect.right, rect.bottom});
        lineTo({rect.left, rect.bottom});
    }
    if (direction == kCCW_Direction){
        lineTo({rect.left, rect.bottom});
        lineTo({rect.right, rect.bottom});
        lineTo({rect.right, rect.top});
    }
}

void GPath::addPolygon(const GPoint pts[], int count){
    moveTo(pts[0]);
    for (int i = 1; i < count; i++){
        lineTo(pts[i]);
    }
}

void GPath::addCircle(GPoint center, float radius, Direction direction){
    float r = radius;
    float cx = center.x;
    float cy = center.y;
    float kr = (4.0f/3.0f) * (sqrt(2) - 1) * r;

    GPoint A = {cx, r+cy};      // 0,1
    GPoint B = {kr+cx, r+cy};   // k,1
    GPoint C = {r+cx, kr+cy};   // 1,k
    GPoint D = {r+cx, cy};      // 1,0
    GPoint E = {r+cx, (-1.0f)*kr+cy};  // 1,-k
    GPoint F = {kr+cx, (-1.0f)*r+cy};  // k,-1
    GPoint G = {cx, (-1.0f)*r+cy};     // 0,-1
    GPoint H = {(-1.0f)*kr+cx, (-1.0f)*r+cy}; // -k,-1
    GPoint I = {(-1.0f)*r+cx, (-1.0f)*kr+cy}; // -1,-k
    GPoint J = {(-1.0f)*r+cx, cy};     // -1,0
    GPoint K = {(-1.0f)*r+cx, kr+cy};  // -1,k
    GPoint L = {(-1.0f)*kr+cx, r+cy};  // -k,1

    moveTo(A);

    if (direction == kCCW_Direction){
        cubicTo(B,C,D);
        cubicTo(E,F,G);
        cubicTo(H,I,J);
        cubicTo(K,L,A);
    }

    if (direction == kCW_Direction){
        cubicTo(L,K,J);
        cubicTo(I,H,G);
        cubicTo(F,E,D);
        cubicTo(C,B,A);
    }
}

GRect GPath::bounds() const {
    if (fPts.empty()){
        return {0,0,0,0};
    }

    float xmax = 0;
    float ymax = 0;
    float xmin = 0;
    float ymin = 0;

    GPoint pts[kMaxNextPoints];
    Edger edger(*this);
    Verb v;  
    
    while ((v = edger.next(pts)) != kDone) {
        switch (v) {
            case kLine:
            {
                xmax = std::max(std::max(xmax, fPts[0].x), fPts[1].x);
                ymax = std::max(std::max(ymax, fPts[0].y), fPts[1].y);
                xmin = std::min(std::min(xmin, fPts[0].x),fPts[1].x);
                ymin = std::min(std::min(ymin, fPts[0].y), fPts[1].y);
            } break;
            case kQuad:
            {
                GPoint A = pts[0];
                GPoint B = pts[1];
                GPoint C = pts[2];

                xmax = std::max(std::max(xmax, A.x), C.x);
                ymax = std::max(std::max(ymax, A.y), C.y);
                xmin = std::min(std::min(xmin, A.x), C.x);
                ymin = std::min(std::min(ymin, A.y), C.y);

                // t when f'(x) == 0
                float tx = (A.x - B.x) / (A.x - 2*B.x + C.x);
                float ty = (A.y - B.y) / (A.y - 2*B.y + C.y);

                if (tx >= 0 && tx <= 1){
                    float x = A.x*(1 - tx)*(1-tx) + 2*B.x*tx*(1 - tx) + C.x*tx*tx; 
                    xmax = std::max(xmax, x);
                    xmin = std::min(xmin, x);
                }
                if (ty >= 0 && ty <= 1){
                    float y = A.y*(1 - ty)*(1-ty) + 2*B.y*ty*(1 - ty) + C.y*ty*ty;
                    ymax = std::max(ymax, y);
                    ymin = std::min(ymin, y);
                }   
            } break;
                
            case kCubic:
            {
                GPoint A = pts[0];
                GPoint B = pts[1];
                GPoint C = pts[2];
                GPoint D = pts[3];   

                xmax = std::max(std::max(xmax, A.x), D.x);
                ymax = std::max(std::max(ymax, A.y), D.y);
                xmin = std::min(std::min(xmin, A.x), D.x);
                ymin = std::min(std::min(ymin, A.y), D.y);

                // t when f'(x) == 0
                float sqrtNx = -sqrt(B.x*B.x - D.x*B.x - B.x*C.x + D.x*A.x + C.x*C.x - A.x*C.x);
                float sqrtPx = sqrt(B.x*B.x - D.x*B.x - B.x*C.x + D.x*A.x + C.x*C.x - A.x*C.x);
                float sqrtNy = -sqrt(B.y*B.y - D.y*B.y - B.y*C.y + D.y*A.y + C.y*C.y - A.y*C.y);
                float sqrtPy = sqrt(B.y*B.y - D.y*B.y - B.y*C.y + D.y*A.y + C.y*C.y - A.y*C.y);

                float txN = (-A.x + 2*B.x - C.x + sqrtNx) / (D.x - A.x + 3*B.x - 3*C.x);
                float txP = (-A.x + 2*B.x - C.x + sqrtPx) / (D.x - A.x + 3*B.x - 3*C.x);
                float tyN = (-A.y + 2*B.y - C.y + sqrtNy) / (D.y - A.y + 3*B.y - 3*C.y);
                float tyP = (-A.y + 2*B.y - C.y + sqrtPy) / (D.y - A.y + 3*B.y - 3*C.y);

                if (txN >= 0 && txN <= 1){
                    float x = A.x*(1-txN)*(1-txN)*(1-txN) + 3*B.x*txN*(1-txN)*(1-txN) +3*C.x*(1-txN)*txN*txN + D.x*txN*txN*txN;
                    xmax = std::max(xmax, x);
                    xmin = std::min(xmin, x);
                }
                if (txP >= 0 && txP <= 1){
                    float x = A.x*(1-txP)*(1-txP)*(1-txP) + 3*B.x*txP*(1-txP)*(1-txP) +3*C.x*(1-txP)*txP*txP + D.x*txP*txP*txP;
                    xmax = std::max(xmax, x);
                    xmin = std::min(xmin, x);
                }
                if (tyN >= 0 && tyN <= 1){
                    float y = A.y*(1-tyN)*(1-tyN)*(1-tyN) + 3*B.y*tyN*(1-tyN)*(1-tyN) +3*C.y*(1-tyN)*tyN*tyN + D.y*tyN*tyN*tyN;
                    ymax = std::max(ymax, y);
                    ymin = std::min(ymin, y);
                }
                if (tyP >= 0 && tyP <= 1){
                    float y = A.y*(1-tyP)*(1-tyP)*(1-tyP) + 3*B.y*tyP*(1-tyP)*(1-tyP) +3*C.y*(1-tyP)*tyP*tyP + D.y*tyP*tyP*tyP;
                    ymax = std::max(ymax, y);
                    ymin = std::min(ymin, y);
                }
            } break;
            default:
                break;
        }
    }
    return {xmin, ymin, xmax, ymax};
}

void GPath::transform(const GMatrix& m) {
    for (int i = 0; i < fPts.size(); i++){
        fPts[i] = m * fPts[i];
    }
}

void GPath::ChopQuadAt(const GPoint *src, GPoint *dst, float t){
    // F(t) = (1-t)A + tB
    // G(t) = (1-t)B + tC
    // H(t) = (1-t)F(x) + tG(t)

    GPoint A = src[0];
    GPoint B = src[1];
    GPoint C = src[2];

    dst[0] = A;
    dst[4] = C;
    dst[1] = (1-t)*A + t*B;
    dst[3] = (1-t)*B + t*C;
    dst[2] = (1-t)*dst[1] + t*dst[3];
}

void GPath::ChopCubicAt(const GPoint *src, GPoint *dst, float t){
    // AB = (1-t)A + tB
        // ABC = (1-t)AB + tBC
    // BC = (1-t)B + tC        // ABCD = (1-t)AB + tCD
        // BCD = (1-t)BC + tCD
    // CD = (1-t)C + tD
    
    GPoint A = src[0];
    GPoint B = src[1];
    GPoint C = src[2];
    GPoint D = src[3];
    GPoint BC = (1-t)*B + t*C;
    
    dst[0] = A;
    dst[6] = D;
    dst[1] = (1-t)*A + t*B; // AB
    dst[5] = (1-t)*C + t*D; // CD
    dst[2] = (1-t)*dst[1] + t*BC; // ABC
    dst[4] = (1-t)*BC + t*dst[5]; // BCD
    dst[3] = (1-t)*dst[2] + t*dst[4];

}

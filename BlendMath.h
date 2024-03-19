#include "include/GPixel.h"
#include "include/GPaint.h"
#include "include/GBlendMode.h"

static inline GPixel kSrcOver(const GPixel src, const GPixel dst){
    int srcA = GPixel_GetA(src);

    int A = srcA + (GPixel_GetA(dst) * (255 - srcA) + 127) / 255;
    int R = GPixel_GetR(src) + (GPixel_GetR(dst) * (255 - srcA) + 127) / 255;
    int G = GPixel_GetG(src) + (GPixel_GetG(dst) * (255 - srcA) + 127) / 255;
    int B = GPixel_GetB(src) + (GPixel_GetB(dst) * (255 - srcA) + 127) / 255;

    return GPixel_PackARGB(A, R, G, B);
}

static inline GPixel kDstOver(const GPixel src, const GPixel dst){
    int dstA = GPixel_GetA(dst);

    int A = dstA + (GPixel_GetA(src) * (255 - dstA) + 127) / 255;
    int R = GPixel_GetR(dst) + (GPixel_GetR(src) * (255 - dstA) + 127) / 255;
    int G = GPixel_GetG(dst) + (GPixel_GetG(src) * (255 - dstA) + 127) / 255;
    int B = GPixel_GetB(dst) + (GPixel_GetB(src) * (255 - dstA) + 127) / 255;
        
    return GPixel_PackARGB(A, R, G, B);
}

static inline GPixel kSrcIn(const GPixel src, const GPixel dst){
    int dstA = GPixel_GetA(dst);

    int A = (dstA * GPixel_GetA(src) + 127) / 255;
    int R = (dstA * GPixel_GetR(src) + 127) / 255;
    int G = (dstA * GPixel_GetG(src) + 127) / 255;
    int B = (dstA * GPixel_GetB(src) + 127) / 255;

    return GPixel_PackARGB(A, R, G, B);
}

static inline GPixel kDstIn(const GPixel src, const GPixel dst){
    int srcA = GPixel_GetA(src);

    int A = (srcA * GPixel_GetA(dst) + 127) / 255;
    int R = (srcA * GPixel_GetR(dst) + 127) / 255;
    int G = (srcA * GPixel_GetG(dst) + 127) / 255;
    int B = (srcA * GPixel_GetB(dst) + 127) / 255;

    return GPixel_PackARGB(A, R, G, B);
}

static inline GPixel kSrcOut(const GPixel src, const GPixel dst){
    int dstA = GPixel_GetA(dst);

    int A = (GPixel_GetA(src) * (255 - dstA) + 127) / 255;
    int R = (GPixel_GetR(src) * (255 - dstA) + 127) / 255;
    int G = (GPixel_GetG(src) * (255 - dstA) + 127) / 255;
    int B = (GPixel_GetB(src) * (255 - dstA) + 127) / 255;
        
    return GPixel_PackARGB(A, R, G, B);
}

static inline GPixel kDstOut(const GPixel src, const GPixel dst){
    int srcA = GPixel_GetA(src);

    int A = (GPixel_GetA(dst) * (255 - srcA) + 127) / 255;
    int R = (GPixel_GetR(dst) * (255 - srcA) + 127) / 255;
    int G = (GPixel_GetG(dst) * (255 - srcA) + 127) / 255;
    int B = (GPixel_GetB(dst) * (255 - srcA) + 127) / 255;
        
    return GPixel_PackARGB(A, R, G, B);
}

static inline GPixel kSrcATop(const GPixel src, const GPixel dst){
    int srcA = GPixel_GetA(src);
    int dstA = GPixel_GetA(dst);

    int A = (dstA * srcA + 127) / 255 + (dstA * (255 - srcA) + 127) / 255;
    int R = (dstA * GPixel_GetR(src) + 127) / 255 + (GPixel_GetR(dst) * (255 - srcA) + 127) / 255;
    int G = (dstA * GPixel_GetG(src) + 127) / 255 + (GPixel_GetG(dst) * (255 - srcA) + 127) / 255;
    int B = (dstA * GPixel_GetB(src) + 127) / 255 + (GPixel_GetB(dst) * (255 - srcA) + 127) / 255;
        
    return GPixel_PackARGB(A, R, G, B);
}

static inline GPixel kDstATop(const GPixel src, const GPixel dst){
    int srcA = GPixel_GetA(src);
    int dstA = GPixel_GetA(dst);

    int A = (srcA * dstA + 127) / 255 + (srcA * (255 - dstA) + 127) / 255;
    int R = (srcA * GPixel_GetR(dst) + 127) / 255 + (GPixel_GetR(src) * (255 - dstA) + 127) / 255;
    int G = (srcA * GPixel_GetG(dst) + 127) / 255 + (GPixel_GetG(src) * (255 - dstA) + 127) / 255;
    int B = (srcA * GPixel_GetB(dst) + 127) / 255 + (GPixel_GetB(src) * (255 - dstA) + 127) / 255;
        
    return GPixel_PackARGB(A, R, G, B);
}

static inline GPixel kXor(const GPixel src, const GPixel dst){
    int srcA = GPixel_GetA(src);
    int dstA = GPixel_GetA(dst);

    int A = ((dstA * (255 - srcA) + srcA * (255 - dstA) + 127) / 255);
    int R = ((GPixel_GetR(dst) * (255 - srcA) + GPixel_GetR(src) * (255 - dstA) + 127) / 255);
    int G = ((GPixel_GetG(dst) * (255 - srcA) + GPixel_GetG(src) * (255 - dstA) + 127) / 255);
    int B = ((GPixel_GetB(dst) * (255 - srcA) + GPixel_GetB(src) * (255 - dstA) + 127) / 255);

    return GPixel_PackARGB(A, R, G, B);
}


static inline GPixel blend(GBlendMode mode, const GPixel src, const GPixel dst) {
    switch (mode) {
        case GBlendMode::kClear: return 0;
        case GBlendMode::kSrc: return src;
        case GBlendMode::kDst: return dst;
        case GBlendMode::kSrcOver: return kSrcOver(src, dst);
        case GBlendMode::kDstOver: return kDstOver(src, dst);
        case GBlendMode::kSrcIn: return kSrcIn(src, dst);
        case GBlendMode::kDstIn: return kDstIn(src, dst);
        case GBlendMode::kSrcOut: return kSrcOut(src, dst);
        case GBlendMode::kDstOut: return kDstOut (src, dst);
        case GBlendMode::kSrcATop: return kSrcATop(src, dst);
        case GBlendMode::kDstATop: return kDstATop(src, dst);
        case GBlendMode::kXor: return kXor(src, dst);  
    }
    return 0;
}
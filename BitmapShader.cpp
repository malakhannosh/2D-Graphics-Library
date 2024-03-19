#include <memory>
#include "include/GPixel.h"
#include "include/GBitmap.h"
#include "include/GShader.h"
#include "include/GMatrix.h"


class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& bitmap, const GMatrix& localInverse, GShader::TileMode mode) {
        this->bm = bitmap;
        this->fInverse = localInverse;
        this->tm = mode;
    }

    ~BitmapShader() override {}

    bool isOpaque() override {
        return isOpaqueFlag;
    }

    bool setContext(const GMatrix& ctm) override {

        if(!ctm.invert(&ctmInv)) {
            return false;
        }
        ctmInv = fInverse * ctmInv;
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint temp = { x + 0.5f, y + 0.5f};
        GPoint p = ctmInv * temp;
        for (int i = 0; i < count; i++){
            float tx = p.x / bm.width();
            float ty = p.y / bm.height();
            switch (tm) {
                case kClamp:
                    tx = std::min(std::max(tx, 0.0f), 1.0f);
                    ty = std::min(std::max(ty, 0.0f), 1.0f);
                    break;
                case kRepeat:
                    tx = tx - floor(tx);
                    ty = ty - floor(ty);
                    break;
                case kMirror:
                    tx = mirror(tx);
                    ty = mirror(ty);
                    break;
            }
            tx *= bm.width();
            ty *= bm.height();
            if(tx == bm.width()){
                tx -= 1;
            }
            if(ty == bm.height()){
                ty -= 1;
            }
            row[i] = *bm.getAddr(GFloorToInt(tx),GFloorToInt(ty));

            p.x += ctmInv[0];
            p.y += ctmInv[3];
        }
    }

    float mirror(float m) {
        m *= 0.5f;
        m = m - floor(m);
        m *= 2.0f;
        if (m > 1) {
            m = 1 - (m-floor(m));
        }
        m *= 0.5f;
        m = m - floor(m);
        if (m > 0.5f) {
            m = 1 - m;
        }
        m *= 2.0f;
        return m;
    }

private:
    bool isOpaqueFlag;
    GMatrix ctmInv;
    GMatrix fInverse;
    GBitmap bm;
    TileMode tm;
};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bm, const GMatrix& localInverse, GShader::TileMode mode) {
    return std::unique_ptr<GShader>(new BitmapShader(bm, localInverse, mode));
}

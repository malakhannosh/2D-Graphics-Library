#include <memory>
#include "include/GPixel.h"
#include "include/GBitmap.h"
#include "include/GShader.h"
#include "include/GMatrix.h"
#include "CanvasHelpers.h"


class TriGradient : public GShader {

    public:
        TriGradient(GColor c0, GColor c1, GColor c2, GMatrix baryM){
            this->c0 = c0;
            this->c1 = c1;
            this->c2 = c2;
            this->isOpaqueFlag = true;
            this->baryM = baryM;
        }
        bool isOpaque() override { return isOpaqueFlag; }

        bool setContext(const GMatrix& ctm) override {
            if(!(ctm*baryM).invert(&fInverse)) {
                return false;
            }
            return true;
            }
        
        void shadeRow(int x, int y, int count, GPixel row[]) override {

            GPoint temp = { x + 0.5f, y + 0.5f };
            GPoint p = fInverse * temp;
            GColor dc1 = c1-c0;
            GColor dc2 = c2-c0;
            GColor c = p.x * dc1 + p.y * dc2 + c0;
            GColor dc = fInverse[0] * dc1 + fInverse[3] * dc2;

            for(int i = 0; i < count; i++){
                row[i] = convert(c);
                c += dc;
            }
        }

    private:
        bool isOpaqueFlag;
        GMatrix baryM;
        GMatrix fInverse;
        GColor c0;
        GColor c1;
        GColor c2;
};

std::unique_ptr<GShader> GCreateTriGradient(GColor c0, GColor c1, GColor c2, GMatrix baryM){
    return std::unique_ptr<GShader>(new TriGradient(c0,c1,c2,baryM));
}
#include <memory>
#include "include/GPixel.h"
#include "include/GBitmap.h"
#include "include/GShader.h"
#include "include/GMatrix.h"
#include "CanvasHelpers.h"


class ComposeShader : public GShader {
    GShader* s0;
    GShader* s1;
    public:
        ComposeShader(GShader* s0, GShader* s1){
            this->s0 = s0;
            this->s1 = s1;
        }

        bool isOpaque() override { return true; }

        bool setContext(const GMatrix& ctm) override {
            // s0->setContext(ctm);
            // s1->setContext(ctm);
            return s0->setContext(ctm) && s1->setContext(ctm);
        }
        
        void shadeRow(int x, int y, int count, GPixel row[]) override {
            std::vector<GPixel> stor0(count);
            s0->shadeRow(x,y,count,&stor0[0]);
            std::vector<GPixel> stor1(count);
            s1->shadeRow(x,y,count,&stor1[0]);

            for(int i = 0; i < count; i++){
                row[i] = modulate(stor0[i], stor1[i]);
            }
        }
};



std::unique_ptr<GShader> GComposeShader(GShader* s0, GShader* s1){
    return std::unique_ptr<GShader>(new ComposeShader(s0, s1));
}

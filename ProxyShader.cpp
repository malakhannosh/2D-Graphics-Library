#include <memory>
#include "include/GPixel.h"
#include "include/GBitmap.h"
#include "include/GShader.h"
#include "include/GMatrix.h"


class ProxyShader : public GShader {
    GShader* fRealShader;
    GMatrix  fExtraTransform;
    public:
        ProxyShader(GShader* shader, const GMatrix& extraTransform)
            : fRealShader(shader), fExtraTransform(extraTransform) {}

        bool isOpaque() override { return fRealShader->isOpaque(); }

        bool setContext(const GMatrix& ctm) override {
            return fRealShader->setContext(ctm * fExtraTransform);
        }
        
        void shadeRow(int x, int y, int count, GPixel row[]) override {
            fRealShader->shadeRow(x, y, count, row);
        }
};

std::unique_ptr<GShader> GCreateProxy(GShader* shader, const GMatrix& extraTransform){
    return std::unique_ptr<GShader>(new ProxyShader(shader, extraTransform));
}

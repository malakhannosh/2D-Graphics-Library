#include <memory>
#include "include/GPixel.h"
#include "include/GBitmap.h"
#include "include/GShader.h"
#include "include/GMatrix.h"
#include "CanvasHelpers.h"

class LinearGradient : public GShader {
    public:
        LinearGradient(const GPoint p0, const GPoint p1, const GColor colors[], int count, GShader::TileMode mode){

            for (int i = 0; i < count; i++){
                this->colors.push_back(colors[i]);
            }
            this->isOpaqueFlag = true;
            this->transformation = GMatrix(p1.x - p0.x, p0.y - p1.y, p0.x, p1.y - p0.y, p1.x - p0.x, p0.y);
            this->tm = mode;
        }

        ~LinearGradient() override {}

        bool isOpaque() override {
            return isOpaqueFlag;
        }

        bool setContext(const GMatrix& ctm) override {
            if(!(ctm*transformation).invert(&fInverse)) {
                return false;
            }
            return true;
        }

        void shadeRow(int x, int y, int count, GPixel row[]) override {
            
            GPoint temp = { x + 0.5f, y + 0.5f };
            GPoint p = fInverse * temp;

            for (int i = 0; i < count; i ++){
                float tx;
                switch (tm) {
                    case kClamp:
                        tx = std::min(std::max(p.x, 0.0f), 1.0f);
                        break;
                    case kRepeat:
                        tx = p.x - floor(p.x);
                        break;
                    case kMirror:
                        tx = mirror(p.x);
                        break;
                }
                tx = tx * (colors.size() - 1);
                int index = floor(tx);
                float t = tx - index;
                GColor c;
                if (index == colors.size() - 1){
                    c = colors[index];
                }
                else {
                    c = (1-t) * colors[index] + t * colors[index+1];
                }
                row[i] = convert(c);
                p.x += fInverse[0];
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
        GMatrix transformation;
        GMatrix fInverse;
        std::vector<GColor> colors;
        TileMode tm;
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode mode){
    return std::unique_ptr<GShader>(new LinearGradient(p0, p1, colors, count, mode));
}

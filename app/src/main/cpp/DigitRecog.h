//
// Created by wangyu on 2019/9/25.
//
#ifndef HANDWRITTENDIGITRECOGDEMO_DIGITRECOG_H
#define HANDWRITTENDIGITRECOGDEMO_DIGITRECOG_H



#include <string>
#include "ncnn/net.h"
#include "modelNetId/LeNet_p27_sim.id.h"
#include <algorithm>

namespace HwDr {
    class DigitRecog {
    public:
        DigitRecog(const std::string &model_path);
        DigitRecog(AAssetManager* mgr);
        ~DigitRecog();
        void start(ncnn::Mat& ncnn_img, std::vector<float>&feature10);
        void SetThreadNum(int threadNum);
    private:
        void DigitRecogNet(ncnn::Mat& img_);
        void normalize(std::vector<float> &feature);
        ncnn::Net DigitRecognet;
        ncnn::Mat ncnn_img;
        std::vector<float> feature_out;
        int threadnum = 2;
    };

}
#endif //HANDWRITTENDIGITRECOGDEMO_DIGITRECOG_H

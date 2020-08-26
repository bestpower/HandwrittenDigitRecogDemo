//
// Created by wangyu on 2019/9/25.
//

#include "DigitRecog.h"


namespace HwDr {

    //载入模型数据
    DigitRecog::DigitRecog(const std::string &model_path) {
        std::string param_files = model_path + "LeNet_p20-sim.param";
        std::string bin_files = model_path + "LeNet_p20-sim.bin";
        DigitRecognet.load_param(param_files.data());
        DigitRecognet.load_model(bin_files.data());
    }

    DigitRecog::~DigitRecog() {
        DigitRecognet.clear();
    }
    void DigitRecog::SetThreadNum(int threadNum) {
        threadnum = threadNum;
    }
    //遮挡模型推理结构
    void DigitRecog::DigitRecogNet(ncnn::Mat& img_) {
        feature_out.resize(10);
        ncnn::Extractor ex = DigitRecognet.create_extractor();
        ex.set_num_threads(threadnum);
        ex.set_light_mode(true);
        ex.input("input.1", img_);     // input node
        ncnn::Mat out;
        ex.extract("45", out);     // output node
        ncnn::Mat test;
        for (int j = 0; j < 10; j++)
        {
            feature_out[j] = out[j];
        }
    }
    //遮挡模型推理方法
    void DigitRecog::start(ncnn::Mat& ncnn_img, std::vector<float>&feature10) {
        DigitRecogNet(ncnn_img);
        feature10 = feature_out;
    }
}
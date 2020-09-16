//
// Created by wangyu on 2019/9/25.
//

#include "DigitRecog.h"

#define TAG "HwDrSo"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))

namespace HwDr {

    //载入模型数据
    DigitRecog::DigitRecog(const std::string &model_path) {
        std::string param_files = model_path + "LeNet_p27_sim.param";
        std::string bin_files = model_path + "LeNet_p27_sim.bin";
        DigitRecognet.load_param(param_files.data());
        DigitRecognet.load_model(bin_files.data());
    }

    //加密方式载入
    static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
    static ncnn::PoolAllocator g_workspace_pool_allocator;
    //加密方式模型载入
    DigitRecog::DigitRecog(AAssetManager *mgr) {
        ncnn::Option opt;
        opt.blob_allocator = &g_blob_pool_allocator;
        opt.workspace_allocator = &g_workspace_pool_allocator;
        DigitRecognet.opt = opt;
        // init param
        {
            int retp = DigitRecognet.load_param_bin(mgr, "Mnist/models/LeNet_p27_sim.param.bin");
            if (retp != 0)
            {
                LOGE("load_param_bin failed");
            }
        }
        // init bin
        {
            int retm = DigitRecognet.load_model(mgr, "Mnist/models/LeNet_p27_sim.bin");
            if (retm != 0)
            {
                LOGE("load_model_bin failed");
            }
        }
    }

    DigitRecog::~DigitRecog() {
        DigitRecognet.clear();
    }
    void DigitRecog::SetThreadNum(int threadNum) {
        threadnum = threadNum;
    }
    //遮挡模型推理结构
    void DigitRecog::DigitRecogNet(ncnn::Mat& img_) {
        ncnn::Extractor ex = DigitRecognet.create_extractor();
        ex.set_num_threads(threadnum);
        ex.set_light_mode(true);
//        ex.input("data", img_);     // input node
        ex.input(LeNet_p27_sim_param_id::BLOB_data, img_);
        ncnn::Mat out;
//        ex.extract("prob", out);     // output node
        ex.extract(LeNet_p27_sim_param_id::BLOB_prob, out);

        // manually call softmax on the fc output
        // convert result into probability
        // skip if your model already has softmax operation
        //概率化处理
        {
            ncnn::Layer* softmax = ncnn::create_layer("Softmax");
            ncnn::ParamDict pd;
            softmax->load_param(pd);
            softmax->forward_inplace(out, DigitRecognet.opt);
            delete softmax;
        }
        out = out.reshape(out.w * out.h * out.c);
        unsigned int out_size = (unsigned int)(out.w);
        feature_out.resize(out_size);
        for (int j = 0; j < out.w; j++)
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
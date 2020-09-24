//
// Created by wangyu on 2019/9/25.
//

#include "DigitRecog.h"

#define TAG "HwDrSo"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))

namespace HwDr {

    static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
    static ncnn::PoolAllocator g_workspace_pool_allocator;

    //载入模型数据
    DigitRecog::DigitRecog(AAssetManager *mgr) {

        ncnn::Option opt;
        opt.blob_allocator = &g_blob_pool_allocator;
        opt.workspace_allocator = &g_workspace_pool_allocator;
        DigitRecognet.opt = opt;

        // init param
        {
            int retp = DigitRecognet.load_param(mgr, "Mnist/models/LeNet_p27_sim.param");
            if (retp != 0)
            {
                LOGE("load_param failed");
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


    //加密方式模型载入
//    DigitRecog::DigitRecog(AAssetManager *mgr) {
//        ncnn::Option opt;
//        opt.blob_allocator = &g_blob_pool_allocator;
//        opt.workspace_allocator = &g_workspace_pool_allocator;
//        DigitRecognet.opt = opt;
//        // init param
//        {
//            int retp = DigitRecognet.load_param_bin(mgr, "Mnist/models/LeNet_p27_sim.param.bin");
//            if (retp != 0)
//            {
//                LOGE("load_param_bin failed");
//            }
//        }
//        // init bin
//        {
//            int retm = DigitRecognet.load_model(mgr, "Mnist/models/LeNet_p27_sim.bin");
//            if (retm != 0)
//            {
//                LOGE("load_model_bin failed");
//            }
//        }
//    }

    DigitRecog::~DigitRecog() {
        DigitRecognet.clear();
    }
    void DigitRecog::SetThreadNum(int threadNum) {
        threadnum = threadNum;
    }
    //遮挡模型推理结构
    unsigned long time_0, time_1;
    void DigitRecog::DigitRecogNet(ncnn::Mat& img_) {
        time_0 = get_current_time();
        ncnn::Extractor ex = DigitRecognet.create_extractor();
        ex.set_num_threads(threadnum);
        ex.set_light_mode(true);
        // input node
//        ex.input("data", img_);//普通方式
        ex.input(LeNet_p27_sim_param_id::BLOB_data, img_);//加密方式
        ncnn::Mat out;
        // output node
//        ex.extract("prob", out);//普通方式
        ex.extract(LeNet_p27_sim_param_id::BLOB_prob, out);//加密方式
        time_1 = get_current_time();
        LOGD("Model inferenced cost %ld", (time_1-time_0)/1000);//计时范围与benchmark对应

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
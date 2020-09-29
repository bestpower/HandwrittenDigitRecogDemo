#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <android/asset_manager_jni.h>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <math.h>

// ncnn
#include "ncnn/net.h"
#include "ncnn/benchmark.h"
//
#include "DigitRecog.h"
#include "ImgFormatConverter.h"
//opencv
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/types_c.h"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/core_c.h"

static ncnn::UnlockedPoolAllocator g_blob_pool_allocator;
static ncnn::PoolAllocator g_workspace_pool_allocator;

using namespace HwDr;

#define TAG "HwDrSo"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))

static DigitRecog *mDigitRecog;

//sdk是否初始化成功
bool detection_sdk_init_ok = false;

extern "C" {

    //模型初始化
    JNIEXPORT jboolean JNICALL
    Java_paxsz_ai_HwDr_MnistAssetModelInit(JNIEnv *env, jobject instance, jobject assetManager) {
        LOGD("JNI开始手写数字识别模型加密方式初始化");
        //如果已初始化则直接返回
        if (detection_sdk_init_ok) {
            LOGD("手写数字识别模型已经导入");
            return jboolean(true);
        }
        jboolean tRet = jboolean(false);
        AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
        mDigitRecog = new DigitRecog(mgr);
        detection_sdk_init_ok = true;
        tRet = jboolean(true);
        return tRet;
    }

	//模型反初始化
	JNIEXPORT jboolean JNICALL
	Java_paxsz_ai_HwDr_MnistModelUnInit(JNIEnv *env, jobject instance) {
		if(!detection_sdk_init_ok){
			LOGD("手写数字识别模型已经释放过或者未初始化");
			return jboolean(true);
		}
		jboolean tDetectionUnInit = jboolean(false);

		delete mDigitRecog;

		detection_sdk_init_ok=false;
		tDetectionUnInit = jboolean(true);
		LOGD("模型初始化锁，重新置零");
		return tDetectionUnInit;

	}


	//设置多线程
	JNIEXPORT jboolean JNICALL
	Java_paxsz_ai_HwDr_SetThreadsNumber(JNIEnv *env, jobject instance, jint threadsNumber) {
		if(!detection_sdk_init_ok){
			LOGD("手写数字识别模型SDK未初始化，直接返回");
			return jboolean(false);
		}
		if(threadsNumber!=1&&threadsNumber!=2&&threadsNumber!=4&&threadsNumber!=8){
			LOGD("线程只能设置1，2，4，8");
			return jboolean(false);
		}
		mDigitRecog->SetThreadNum(threadsNumber);

		return jboolean(true);
	}

	//获取数字手写识别概率数据
	JNIEXPORT jfloatArray JNICALL
	Java_paxsz_ai_HwDr_HwDigitRecog(JNIEnv *env, jobject instance, jbyteArray digitImgData_, jint w, jint h) {
        //字节数组预处理
		jbyte *digitImgData = env->GetByteArrayElements(digitImgData_, NULL);
		unsigned char *digitImgCharData = (unsigned char *) digitImgData;
		//转换图片数据格式
		ncnn::Mat ncnn_img = ncnn::Mat::from_pixels_resize(digitImgCharData, ncnn::Mat::PIXEL_RGBA2GRAY, w, h, 28, 28);
		//输入数据归一化
		const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
		ncnn_img.substract_mean_normalize(0, norm_vals);

		std::vector<float> feature;
		//数字手写识别推理
		mDigitRecog->start(ncnn_img, feature);

		//提取并赋值概率数组
        float featureInfo[10];
		for(int i = 0;i<10;i++){
			featureInfo[i] = feature[i];
		}
		jfloatArray featureArray = env->NewFloatArray(10);
		env->SetFloatArrayRegion(featureArray,0,10,featureInfo);
		//释放资源
		env->ReleaseByteArrayElements(digitImgData_, digitImgData, 0);
        ncnn_img.release();
        std::vector<float>().swap(feature);
		//返回预测结果概率数组
		return featureArray;
	}

    JNIEXPORT jfloatArray JNICALL
    Java_paxsz_ai_HwDr_HwDigitRecogFromBitmap(JNIEnv *env, jobject instance, jobject digitImgBitmap) {
        //转换图片数据格式
        ncnn::Mat ncnn_img = ncnn::Mat::from_android_bitmap_resize(env, digitImgBitmap, ncnn::Mat::PIXEL_BGRA2GRAY, 28, 28);
        //输入数据归一化
        const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
        ncnn_img.substract_mean_normalize(0, norm_vals);

        std::vector<float> feature;
        //数字手写识别推理
        mDigitRecog->start(ncnn_img, feature);

        //提取并赋值概率数组
        float featureInfo[10];
        for(int i = 0;i<10;i++){
            featureInfo[i] = feature[i];
        }
        jfloatArray featureArray = env->NewFloatArray(10);
        env->SetFloatArrayRegion(featureArray,0,10,featureInfo);
		//释放资源
		env->DeleteLocalRef(digitImgBitmap);
        ncnn_img.release();
        std::vector<float>().swap(feature);
        //返回预测结果概率数组
        return featureArray;
    }

    JNIEXPORT jfloatArray JNICALL
    Java_paxsz_ai_HwDr_HwDigitRecogFromPath(JNIEnv *env, jobject instance, jstring imgPath) {
        //路径输入处理
        const char *digitImgPath = env->GetStringUTFChars(imgPath, 0);
        std::string imgPath_ = digitImgPath;
        cv::Mat digitImageMat = cv::imread(imgPath_);
        //转换图片数据格式
        ncnn::Mat ncnn_img = ncnn::Mat::from_pixels_resize(digitImageMat.data, ncnn::Mat::PIXEL_BGR2GRAY, digitImageMat.cols, digitImageMat.rows, 28, 28);
        //输入数据归一化
        const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
        ncnn_img.substract_mean_normalize(0, norm_vals);

        std::vector<float> feature;
        //数字手写识别推理
        mDigitRecog->start(ncnn_img, feature);

        //提取并赋值概率数组
        float featureInfo[10];
        for(int i = 0;i<10;i++){
            featureInfo[i] = feature[i];
        }
        jfloatArray featureArray = env->NewFloatArray(10);
        env->SetFloatArrayRegion(featureArray,0,10,featureInfo);
        //释放资源
        env->ReleaseStringUTFChars(imgPath, digitImgPath);
        ncnn_img.release();
        std::vector<float>().swap(feature);
        //返回预测结果概率数组
        return featureArray;
    }
}
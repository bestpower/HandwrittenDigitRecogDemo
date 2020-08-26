#include <android/bitmap.h>
#include <android/log.h>
#include <jni.h>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <math.h>

// ncnn
#include "net.h"
#include "benchmark.h"
#include "DigitRecog.h"
#include "QualityDetect.h"
#include "RemoveBackgroud.h"
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
//#define ASSERT(status, ret)     if (!(status)) { return ret; }
//#define ASSERT_FALSE(status)    ASSERT(status, false)

static DigitRecog *mDigitRecog;

//sdk是否初始化成功
bool detection_sdk_init_ok = false;

extern "C" {

	JNIEXPORT jboolean JNICALL
	Java_paxsz_ai_HwDr_MnistModelInit(JNIEnv *env, jobject instance, jstring MnistModelPath_) {
		 LOGD("JNI开始人脸检测模型初始化");
		//如果已初始化则直接返回
		if (detection_sdk_init_ok) {
			//  LOGD("人脸检测模型已经导入");
			return true;
		}
		jboolean tRet = false;
		if (NULL == MnistModelPath_) {
			//   LOGD("导入的人脸检测的目录为空");
			return tRet;
		}

		//获取MMNIST模型的绝对路径的目录（不是/aaa/bbb.bin这样的路径，是/aaa/)
		const char *MnistModelPath = env->GetStringUTFChars(MnistModelPath_, 0);
		if (NULL == MnistModelPath) {
			return tRet;
		}

		std::string tMnistModelDir = MnistModelPath;
        std::string tLastChar = tMnistModelDir.substr(tMnistModelDir.length() - 1, 1);
		//目录补齐/
		if ("\\" == tLastChar) {
			tMnistModelDir = tMnistModelDir.substr(0, tMnistModelDir.length() - 1) + "/";
		} else if (tLastChar != "/") {
			tMnistModelDir += "/";
		}

		// use vulkan compute
	//    ncnn::Option opt;
	//    opt.lightmode = true;
	//    opt.num_threads = 4;
	//    opt.blob_allocator = &g_blob_pool_allocator;
	//    opt.workspace_allocator = &g_workspace_pool_allocator;
	//    LOGD("use_vulkan_compute: count = %d", ncnn::get_gpu_count());
	//    if (ncnn::get_gpu_count() != 0) {
	//        opt.use_vulkan_compute = true;
	//    }

		mDigitRecog = new DigitRecog(tMnistModelDir);

		env->ReleaseStringUTFChars(MnistModelPath_, MnistModelPath);
		detection_sdk_init_ok = true;
		tRet = true;
		return tRet;
	}

	//模型反初始化
	JNIEXPORT jboolean JNICALL
	Java_paxsz_ai_HwDr_MnistModelUnInit(JNIEnv *env, jobject instance) {
		if(!detection_sdk_init_ok){
			LOGD("人脸检测MTCNN模型已经释放过或者未初始化");
			return true;
		}
		jboolean tDetectionUnInit = false;

		delete mDigitRecog;

		detection_sdk_init_ok=false;
		tDetectionUnInit = true;
		LOGD("人脸检测初始化锁，重新置零");
		return tDetectionUnInit;

	}


	//设置多线程
	JNIEXPORT jboolean JNICALL
	Java_paxsz_ai_HwDr_SetThreadsNumber(JNIEnv *env, jobject instance, jint threadsNumber) {
		if(!detection_sdk_init_ok){
			LOGD("人脸检测MTCNN模型SDK未初始化，直接返回");
			return false;
		}

		if(threadsNumber!=1&&threadsNumber!=2&&threadsNumber!=4&&threadsNumber!=8){
			LOGD("线程只能设置1，2，4，8");
			return false;
		}

		mDigitRecog->SetThreadNum(threadsNumber);

		return  true;
	}


//	JNIEXPORT jboolean JNICALL
//	Java_paxsz_ai_HwDr_SetTimeCount(JNIEnv *env, jobject instance, jint timeCount) {
//
//		if(!detection_sdk_init_ok){
//			LOGD("人脸检测MTCNN模型SDK未初始化，直接返回");
//			return false;
//		}
//
//		mDigitRecog->SetTimeCount(timeCount);
//
//		return true;
//
//	}

	//获取数字手写识别概率数据
	JNIEXPORT jfloatArray JNICALL
	Java_paxsz_ai_HwDr_HwDigitRecog(JNIEnv *env, jobject instance, jbyteArray digitImgData_, jint w, jint h) {

		jbyte *digitImgData = env->GetByteArrayElements(digitImgData_, NULL);
		unsigned char *digitImgCharData = (unsigned char *) digitImgData;
		//转换图片数据格式
		ncnn::Mat ncnn_img = ncnn::Mat::from_pixels_resize(digitImgCharData, ncnn::Mat::PIXEL_RGBA2GRAY, w, h, 28, 28);
		//输入数据归一化
	//    const float normalize[3] = {1/255.f, 1/255.f, 1/255.f};
	//    ncnn_img.substract_mean_normalize(NULL, normalize);

		const float mean_vals[1] = {0.5f*255.f};
		const float norm_vals[1] = {1/0.5f/255.f};
		ncnn_img.substract_mean_normalize(mean_vals, norm_vals);

		std::vector<float> feature;
		//数字手写识别推理
		mDigitRecog->start(ncnn_img, feature);

		//提取并赋值概率数组
		float *featureInfo = new float[10];
		for(int i = 0;i<10;i++){
			featureInfo[i] = feature[i];
		}
		jfloatArray featureArray = env->NewFloatArray(10);
		env->SetFloatArrayRegion(featureArray,0,10,featureInfo);
		//
		env->ReleaseByteArrayElements(digitImgData_, digitImgData, 0);
		//返回预测结果概率数组
		return featureArray;
	}

}
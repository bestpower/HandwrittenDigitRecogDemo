//
// Created by wangyu on 2020/8/31.
//

#ifndef HANDWRITTENDIGITRECOGDEMO_IMGFORMATCONVERTER_H
#define HANDWRITTENDIGITRECOGDEMO_IMGFORMATCONVERTER_H


//c
#include <string>
#include <algorithm>
#include <jni.h>
//android
#include <android/bitmap.h>
#include <android/log.h>
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


namespace HwDr {
    unsigned long get_current_time(void);
    bool BitmapToMatrix(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix);
    bool MatrixToBitmap(JNIEnv * env, cv::Mat & matrix, jobject obj_bitmap);
    cv::Mat bytesToMat(unsigned char *bytesData, int w, int h, int c);
    unsigned char* MatToBytes(cv::Mat mat);
    bool rgbToBin(JNIEnv * env, jobject obj_bitmap, cv::Mat target_mat, jint binThreshold);
}


#endif //HANDWRITTENDIGITRECOGDEMO_IMGFORMATCONVERTER_H

//
// Created by wangyu on 2019/10/23.
//

#pragma once
#ifndef MY_OCC_DEMO_QUALITYDETECT_H
#define MY_OCC_DEMO_QUALITYDETECT_H
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


namespace FaceIr {
    bool BitmapToMatrix(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix);
    bool MatrixToBitmap(JNIEnv * env, cv::Mat & matrix, jobject obj_bitmap);
    double ImgAver(IplImage *img);
    double ImgVarc(IplImage *img);
    double* getLight(IplImage *image);

    double ssim(cv::Mat &i1, cv::Mat &i2);
    double nrss(cv::Mat &image);
    //运动模糊
    double blurDetect(cv::Mat srcImage);
    //对焦模糊
    double tenengradDetect(cv::Mat srcImage);
}

#endif //MY_OCC_DEMO_QUALITYDETECT_H

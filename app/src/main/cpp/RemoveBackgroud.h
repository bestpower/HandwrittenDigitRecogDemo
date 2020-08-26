//
// Created by wangyu on 2020/1/17.
//
#pragma once
#ifndef MY_IR_FACE_DEMO_REMOVEBACKGROUD_H
#define MY_IR_FACE_DEMO_REMOVEBACKGROUD_H

//c
#include <string>
#include <algorithm>
#include <jni.h>
#include <iostream>
#include <android/log.h>
#include <vector>
#include <cstring>
#include <math.h>
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
    cv::Mat bytesToMat(unsigned char *bytesData, int w, int h, int c);
    unsigned char* MatToBytes(cv::Mat mat);
    cv::Mat createMask(cv::Mat oldMat, int* outlineData);
}

#endif //MY_IR_FACE_DEMO_REMOVEBACKGROUD_H

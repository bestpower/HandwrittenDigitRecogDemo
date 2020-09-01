//
// Created by wangyu on 2020/8/31.
//

#include "ImgFormatConverter.h"

namespace HwDr {

    //bitmap2Mat
    #define ASSERT(status, ret)     if (!(status)) { return ret; }
    #define ASSERT_FALSE(status)    ASSERT(status, false)
        //LOG
    #define TAG "ImgFormatConverter"
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
    #define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))

    bool BitmapToMatrix(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix){
        // 保存图片像素数据
        void * bitmapPixels;
        // 保存图片参数
        AndroidBitmapInfo bitmapInfo;
        // 获取图片参数
        ASSERT_FALSE( AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);
        // 只支持 ARGB_8888 和 RGB_565
        ASSERT_FALSE( bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                      || bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565 );
        // 获取图片像素（锁定内存块）
        ASSERT_FALSE( AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0 );
        ASSERT_FALSE( bitmapPixels );
        if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            // 建立临时 mat
            cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);
            // 拷贝到目标 matrix
            //cv::cvtColor(tmp, matrix, cv::COLOR_BGRA2RGB);
            tmp.copyTo(matrix);
        } else {
            cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
            cv::cvtColor(tmp, matrix, cv::COLOR_BGR5652RGB);
        }
        // 解锁
        AndroidBitmap_unlockPixels(env, obj_bitmap);
        return true;
    }
    bool MatrixToBitmap(JNIEnv * env, cv::Mat & matrix, jobject obj_bitmap){
        void * bitmapPixels;// 保存图片像素数据
        AndroidBitmapInfo bitmapInfo;// 保存图片参数

        ASSERT_FALSE( AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);// 获取图片参数
        ASSERT_FALSE( bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                      || bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565 );// 只支持 ARGB_8888 和 RGB_565
        ASSERT_FALSE( matrix.dims == 2
                      && bitmapInfo.height == (uint32_t)matrix.rows
                      && bitmapInfo.width == (uint32_t)matrix.cols );// 必须是 2 维矩阵，长宽一致
        ASSERT_FALSE( matrix.type() == CV_8UC1 || matrix.type() == CV_8UC3 || matrix.type() == CV_8UC4 );
        ASSERT_FALSE( AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0 );  // 获取图片像素（锁定内存块）
        ASSERT_FALSE( bitmapPixels );

        if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);
            switch (matrix.type()) {
                case CV_8UC1:
                    cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2RGBA);
                    break;
                case CV_8UC3:
                    cv::cvtColor(matrix, tmp, cv::COLOR_RGB2RGBA);
                    break;
                case CV_8UC4:
                    matrix.copyTo(tmp);
                    break;
                default:
                    AndroidBitmap_unlockPixels(env, obj_bitmap);
                    return false;
            }
        } else {
            cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
            switch (matrix.type()) {
                case CV_8UC1:
                    cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2BGR565);
                    break;
                case CV_8UC3:
                    cv::cvtColor(matrix, tmp, cv::COLOR_RGB2BGR565);
                    break;
                case CV_8UC4:
                    cv::cvtColor(matrix, tmp, cv::COLOR_RGBA2BGR565);
                    break;
                default:
                    AndroidBitmap_unlockPixels(env, obj_bitmap);
                    return false;
            }
        }
        AndroidBitmap_unlockPixels(env, obj_bitmap);// 解锁
        return true;
    }
    cv::Mat bytesToMat(unsigned char *bytesData, int w, int h, int c){
        size_t nByte = (size_t)h * w * c;//字节计算
        int nType = CV_8UC4;//图片格式
        cv::Mat faceImageMat = cv::Mat::zeros(h, w, nType);
        memcpy(faceImageMat.data, bytesData, nByte);
        //释放资源
        //delete[] bytesData;
        return faceImageMat;
    }
    unsigned char* MatToBytes(cv::Mat mat){
        int nFlag = mat.channels();
        int nHeight = mat.rows;
        int nWidth = mat.cols;
        size_t nBytes = (size_t)nHeight * nWidth * nFlag;//图像总的字节
        unsigned char* bytesData = new unsigned char[nBytes];//new的单位为字节
        memcpy(bytesData, mat.data, nBytes);//转化函数,注意Mat的data成员
        //释放资源
        //mat.release();
        return bytesData;
    }

    bool rgbToBin(JNIEnv * env, jobject obj_bitmap, cv::Mat target_mat, jint binThreshold)
    {
        cv::Mat matBitmap;
        // Bitmap 转 cv::Mat
        bool ret = BitmapToMatrix(env, obj_bitmap, matBitmap);
        jboolean status = (jboolean)ret;
        if (ret == false) {
            //返回假
            return status;
        }else{
            bool ret1 = false;
            if(matBitmap.channels()>1) {
//                cv::Mat imageGray;
                cv::cvtColor(matBitmap, target_mat, cv::COLOR_RGBA2GRAY);
//                cv::Mat imageBin;
//                threshold(imageGray, target_mat, binThreshold, 255, cv::THRESH_BINARY);
                ret1 = true;
                status = (jboolean) ret1;
                //释放
//                imageGray.release();
            }else{
//                cv::Mat imageBin;
//                threshold(matBitmap, target_mat, binThreshold, 255, cv::THRESH_BINARY);
                target_mat = matBitmap.clone();
                ret1 = true;
                status = (jboolean) ret1;
            }
            //释放资源
            env->DeleteLocalRef(obj_bitmap);
            matBitmap.release();

            return status;
        }
    }
}
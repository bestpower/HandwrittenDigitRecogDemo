//
// Created by wangyu on 2020/1/17.
//

#include "RemoveBackgroud.h"

namespace FaceIr {
    //byte[]->Mat
    cv::Mat bytesToMat(unsigned char *bytesData, int w, int h, int c){
        size_t nByte = (size_t)h * w * c;//字节计算
        int nType = CV_8UC4;//图片格式
        cv::Mat faceImageMat = cv::Mat::zeros(h, w, nType);
        memcpy(faceImageMat.data, bytesData, nByte);
        //释放资源
        //delete[] bytesData;
        return faceImageMat;
    }
    //Mat->byte[]
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
    //去背景
    cv::Mat createMask(cv::Mat oldMat, int* outlineData){
        //1.定义轮廓坐标数组
        std::vector<cv::Point2i> pts;
        for(int i = 0; i < 19; i++){
            pts.push_back(cv::Point2i(outlineData[2*i], outlineData[2*i+1]));
        }
        //输出Mat
        cv::Mat newMat = cv::Mat::zeros(oldMat.size(), CV_8UC4);
        //画布Mat
        cv::Mat mask = cv::Mat::zeros(oldMat.size(), CV_8UC1);
        //2.绘制轮廓多边形
        //const cv::Point* pts[] = {points[0]};
        //int npts[] = {19};
        //cv::polylines(mask, pts, npts, 1, true, cv::Scalar(0,0,0), 1, 8, 0);
        cv::polylines(mask, pts, false, cv::Scalar(0,0,0,255), 1, cv::LINE_8, 0);
        //3.填充多边形
        std::vector<std::vector<cv::Point2i>> vpts;
        vpts.push_back(pts);
        //cv::fillPoly(mask, pts, npts, 1, cv::Scalar(255,255,255), 8, 0, cv::Point());
        //cv::fillPoly(mask, pts, npts, 1, cv::Scalar(255,255,255), 8, 0);
        cv::fillPoly(mask, vpts, cv::Scalar(255,255,255,255), cv::LINE_8, 0, cv::Point());
        //4.图像与操作
        //cv::Mat faceImageMat0;
        //cv::bitwise_and(oldMat, mask, newMat);
        cv::bitwise_and(oldMat, oldMat, newMat, mask);
        //释放资源
        oldMat.release();
        mask.release();
        return newMat;
    }
}
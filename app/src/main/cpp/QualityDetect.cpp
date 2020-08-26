//
// Created by wangyu on 2019/10/23.
//

#include "QualityDetect.h"

namespace FaceIr {

    //bitmap2Mat
    #define ASSERT(status, ret)     if (!(status)) { return ret; }
    #define ASSERT_FALSE(status)    ASSERT(status, false)
    //LOG
    #define TAG "QualityDetectSo"
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
    #define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))
    //位图转cv::Mat格式
    bool BitmapToMatrix(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix) {
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

    //cv::Mat转Bitmap
    bool MatrixToBitmap(JNIEnv * env, cv::Mat & matrix, jobject obj_bitmap) {

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

    //求图像均值（备用方法）
    double ImgAver(IplImage *img)
    {
        int i,j;//循环变量
        int height=img->height;
        int width=img->width;
        int step=img->widthStep/sizeof(uchar);
        uchar *data=(uchar*)img->imageData;

        double aver=0.0;

        for(i=0;i<height;i++)
        {
            for(j=0;j<width;j++)
            {
                aver += data[i*step+j];
            }
        }
        aver=1.0*aver/(height*width);

        return aver;
    }
    //求图像方差（备用方法）
    double ImgVarc(IplImage *img)
    {
        int i,j;//循环变量
        int height=img->height;
        int width=img->width;
        int step=img->widthStep/sizeof(uchar);
        uchar *data=(uchar*)img->imageData;

        double var=0;

        for(i=0;i<height;i++)
        {
            for(j=0;j<width;j++)
            {
                var+=pow((double)data[i*step+j],2);
            }
        }
        var=1.0*var/(height*width);
        var=var-(ImgAver(img)*ImgAver(img));
        var=sqrt(var);

        return var;
    }
    //图片亮度检测
    double* getLight(IplImage *image)
    {
        IplImage *gray = cvCreateImage(cvGetSize(image), image->depth, 1);
        //LOGD("亮度检测图片深度：%d", image->depth);
        cvCvtColor(image, gray, CV_BGR2GRAY);
        int width = gray->width;
        int height = gray->height;
        int s = width * height;//灰度有效点数
        //LOGD("亮度检测图片尺寸：%d %d", width, height);
        double sum = 0;
        double avg = 0;
        CvScalar scalar;
        int ls[256];
        for(int i=0; i<256; i++)
        {
            ls[i]=0;
        }
        for(int i=0;i<height;i++)
        {
            for(int j=0;j<width;j++)
            {
                scalar = cvGet2D(gray, i, j);
                if(scalar.val[0]!=0){
                    //有效点
                    sum += (scalar.val[0] - 128);
                    int x= (int)scalar.val[0];
                    ls[x]++;
                }else{
                    //全黑点
                    s--;
                }
            }
        }
        //LOGD("亮度检测有效点比例 usefulProportion: %f", s/(double)(width * height));
        //LOGD("亮度检测中间值 sum: %f", sum);
        avg = sum / s;
        //LOGD("亮度检测中间值 avg: %f", avg);
        double total = 0;
        double mean = 0;
        for(int i=0;i<256;i++)
        {
            total += abs(i-128-avg) * ls[i];
        }
        //LOGD("亮度检测中间值 total: %f", total);
        mean = total / s;
        //LOGD("亮度检测中间值 mean: %f", mean);
        double cast = abs(avg / mean);
        //LOGD("亮度检测中间值 cast %f", cast);
        double* value = new double[2];
        value[0] = cast;
        value[1] = avg;
        //亮度值（cast<1:正常 cast>1：偏亮或偏暗）
        // avg>0：偏亮 avg<0：偏暗
        return value;
    }

    //图片清晰度检测
    /*double ssim(cv::Mat &i1, cv::Mat &i2)
    {
        const double C1 = 6.5025, C2 = 58.5225;
        int d = CV_32F;
        cv::Mat I1, I2;
        i1.convertTo(I1, d);
        i2.convertTo(I2, d);
        cv::Mat I1_2 = I1.mul(I1);
        cv::Mat I2_2 = I2.mul(I2);
        cv::Mat I1_I2 = I1.mul(I2);
        cv::Mat mu1, mu2;
        GaussianBlur(I1, mu1, cv::Size(11, 11), 1.5);
        GaussianBlur(I2, mu2, cv::Size(11, 11), 1.5);
        cv::Mat mu1_2 = mu1.mul(mu1);
        cv::Mat mu2_2 = mu2.mul(mu2);
        cv::Mat mu1_mu2 = mu1.mul(mu2);
        cv::Mat sigam1_2, sigam2_2, sigam12;
        GaussianBlur(I1_2, sigam1_2, cv::Size(11, 11), 1.5);
        sigam1_2 -= mu1_2;
        GaussianBlur(I2_2, sigam2_2, cv::Size(11, 11), 1.5);
        sigam2_2 -= mu2_2;
        GaussianBlur(I1_I2, sigam12, cv::Size(11, 11), 1.5);
        sigam12 -= mu1_mu2;
        cv::Mat t1, t2, t3;
        t1 = 2 * mu1_mu2 + C1;
        t2 = 2 * sigam12 + C2;
        t3 = t1.mul(t2);

        t1 = mu1_2 + mu2_2 + C1;
        t2 = sigam1_2 + sigam2_2 + C2;
        t1 = t1.mul(t2);

        cv::Mat ssim_map;
        divide(t3, t1, ssim_map);

        //cv:Scale mssim = cv::mean(ssim_map);
        cv::Scalar mssim = cv::mean(ssim_map);

        double ssimValue = (mssim.val[0] + mssim.val[1] + mssim.val[2]) / 3;
        return ssimValue;
    }

    double nrss(cv::Mat &image)
    {
        assert(image.empty());

        cv::Mat gray_img, Ir, G, Gr;
        //if (image.channels() == 3) {
        //    cv::cvtColor(image, gray_img, CV_BGR2GRAY);
        //}
        cv::cvtColor(image, gray_img, CV_RGBA2GRAY);

        //cv::GaussianBlur(gray_img, Ir, cv::Size(7, 7), 0);
        cv::GaussianBlur(gray_img, Ir, cv::Size(7, 7), 6, 6);

        cv::Sobel(gray_img, G, CV_32FC1, 1, 1);
        //cv::Sobel(gray_img, G, CV_16S, 1, 0);
        cv::Sobel(Ir, Gr, CV_32FC1, 1, 1);
        //cv::Sobel(Ir, Gr, CV_16S, 0, 1);

        int block_cols = G.cols * 2 / 9;
        int block_rows = G.rows * 2 / 9;

        cv::Mat best_G, best_Gr;
        float max_stddev = 0.f;
        int pos = 0;
        for (int i = 0; i < 64; i++) {
            int left_x = (i % 8)*(block_cols / 2);
            int left_y = (i / 8)*(block_rows / 2);
            int right_x = left_x + block_cols;
            int right_y = left_y + block_rows;

            if (left_x < 0) left_x = 0;
            if (left_y < 0) left_y = 0;
            if (right_x >= G.cols) right_x = G.cols - 1;
            if (right_y >= G.rows) right_y = G.rows - 1;

            cv::Rect roi(left_x, left_y, right_x-left_x, right_y-left_y);
            cv::Mat temp=G(roi).clone();
            cv::Scalar mean, stddev;
            cv::meanStdDev(temp, mean, stddev);
            if (stddev.val[0] > max_stddev) {
                max_stddev = static_cast<float>(stddev.val[0]);
                pos = i;
                best_G = temp;
                best_Gr = Gr(roi).clone();
            }
        }
        double result = ssim(best_G, best_Gr);
        return result;
    }*/

    //清晰度检测（针对运动模糊）
    double blurDetect(cv::Mat srcImage){

        cv::Mat srcBlur, gray1, gray2, gray3, dstImage;
        double thre = 300; //控制阈值
        //pyrDown(srcImage, dstImage, Size(srcImage.cols/2, srcImage.rows/2));
        cv::GaussianBlur(srcImage, srcBlur, cv::Size(3, 3), 0, 0, cv::BORDER_DEFAULT); //高斯滤波
        cv::convertScaleAbs(srcBlur, srcImage); //使用线性变换转换输入数组元素成8位无符号整型 归一化为0-255
        if (srcImage.channels() != 1)
        {
            //进行灰度化
            cv::cvtColor(srcImage, gray1, cv::COLOR_RGBA2GRAY);
        } else
        {
            gray1 = srcImage.clone();
        }

        cv::Mat tmp_m1, tmp_sd1;    //用来存储均值和方差
        double m1 = 0, sd1 = 0;
        //使用3x3的Laplacian算子卷积滤波
        cv::Laplacian(gray1, gray2, CV_16S, 3, 1, 0, cv::BORDER_DEFAULT);

        /*double minVal, maxVal;
        minMaxLoc(gray2, &minVal, &maxVal);
        double alpha = 255 / (maxVal - minVal), beta = -255 * minVal / (maxVal - minVal);*/

        //归到0~255
        cv::convertScaleAbs(gray2, gray3);
        //计算均值和方差
        cv::meanStdDev(gray3, tmp_m1, tmp_sd1);
        m1 = tmp_m1.at<double>(0, 0);     //均值
        sd1 = tmp_sd1.at<double>(0, 0);   //标准差
        double blurPer = sd1*sd1; //方差
        return blurPer;
    }

    //高斯模糊
    double tenengradDetect(cv::Mat srcImage){

        cv::Mat imageGray;
        cv::cvtColor(srcImage, imageGray, CV_RGBA2GRAY);
        cv::Mat imageSobel;
        cv::Sobel(imageGray, imageSobel, CV_16U, 1, 1);
        //图像的平均灰度
        double meanValue = 0.0;
        meanValue = mean(imageSobel)[0];

        return meanValue;
    }

}
#pragma once

#include <opencv2/opencv.hpp>
#include "ports/IFrameSource.hpp"
// OpencvCameraSource 摄像头采集模块

class OpencvCameraSource : public IFrameSource
{
    public:
        // 构造函数，默认使用摄像头索引为0
        // explicit关键字用于防止隐式类型转换，确保只有明确的构造函数调用才会创建对象
        explicit OpencvCameraSource(int cameraIndex = 0);

        bool open() override; // 打开摄像头

        bool read(cv::Mat& frame) override; // 读取一帧图像

        bool isOpened() const override; // 检查摄像头是否已打开
        
        bool close() override; // 关闭摄像头


    private:
        int cameraIndex_; // 摄像头索引

        cv::VideoCapture cap_; // OpenCV的视频捕获对象
};

class NetworkCameraSource : public IFrameSource
{
    public:
        explicit NetworkCameraSource(const std::string& url);

        bool open() override; // 打开网络摄像头

        bool read(cv::Mat& frame) override; // 读取一帧图像

        bool isOpened() const override; // 检查网络摄像头是否已打开
        
        bool close() override; // 关闭网络摄像头
    private:
        std::string url_;
        cv::VideoCapture cap_;
};
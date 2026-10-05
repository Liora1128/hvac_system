#pragma once

#include "ports/IFrameSource.hpp"
#include <opencv2/opencv.hpp>
#include <string>

class VideoFileSource : public IFrameSource
{
    public:
        explicit VideoFileSource(const std::string& filePath);

        bool open() override; // 打开视频文件
        bool read(cv::Mat& frame) override; // 读取一帧图像
        bool isOpened() const override; // 检查视频文件是否已打开
        bool close() override; // 关闭视频文件
    private:
        std::string filePath_;
        cv::VideoCapture videoCapture_;
};
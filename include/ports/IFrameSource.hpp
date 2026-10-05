#pragma  once

#include <opencv2/core.hpp>

// IFrameSource 接口类，定义了摄像头采集模块的基本功能
class IFrameSource
{
    public:
        virtual ~IFrameSource() = default;

        // 纯虚函数，由具体的摄像头采集模块实现
        virtual bool open() = 0;
        virtual bool read(cv::Mat& frame) = 0;
        virtual bool isOpened() const = 0;
        virtual bool close() = 0;
};
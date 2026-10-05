#include "camera/OpenCvCameraSource.hpp"

// 构造函数，初始化摄像头索引
OpencvCameraSource::OpencvCameraSource(int cameraIndex)
    : cameraIndex_(cameraIndex)
{
}

// 打开摄像头
bool OpencvCameraSource::open()
{
    cap_.open(cameraIndex_);
    return cap_.isOpened();
}

// 读取一帧图像
bool OpencvCameraSource::read(cv::Mat& frame)
{
    if (!cap_.isOpened())
    {
        return false;
    }
    return cap_.read(frame);
}

// 检查摄像头是否已打开
bool OpencvCameraSource::isOpened() const
{
    return cap_.isOpened();
}

// 关闭摄像头
bool OpencvCameraSource::close()
{
    if (cap_.isOpened())
    {
        cap_.release();
        return true;
    }
    return false;
}

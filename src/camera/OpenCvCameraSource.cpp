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

// NetworkCameraSource 类的实现
NetworkCameraSource::NetworkCameraSource(const std::string& url)
    : url_(url)
{
}

bool NetworkCameraSource::open()
{
    cap_.release(); // 释放之前的资源

    _putenv_s(
        "OPENCV_FFMPEG_CAPTURE_OPTIONS",
        "rtsp_transport;tcp|fflags;nobuffer|max_delay;0"
    );

    if(!cap_.open(url_), cv::CAP_FFMPEG)
    {
        std::cerr << "[NetworkCamera] Failed to open stream." << url_ << std::endl;
        return false;
    }
    cap_.set(cv::CAP_PROP_BUFFERSIZE, 1); // 设置缓冲区大小为1，减少延迟
    std::cout
        << "[NetworkCamera] Stream opened successfully."
        << std::endl;

    return true;
}

bool NetworkCameraSource::read(cv::Mat& frame)
{
    if (!cap_.isOpened())
    {
        return false;
    }

    return cap_.read(frame) && !frame.empty();
}

bool NetworkCameraSource::isOpened() const
{
    return cap_.isOpened();
}

bool NetworkCameraSource::close()
{
    if (!cap_.isOpened())
    {
        return false;
    }

    cap_.release();
    return true;
}
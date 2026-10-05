#include "video/VideoFileSource.hpp"

VideoFileSource::VideoFileSource(const std::string& filePath)
    : filePath_(filePath)
{
}

bool VideoFileSource::open()
{
    videoCapture_.open(filePath_);
    return videoCapture_.isOpened();
}

bool VideoFileSource::read(cv::Mat& frame)
{
    if(!videoCapture_.isOpened())
    {
        return false;
    }
    return videoCapture_.read(frame);
}

bool VideoFileSource::isOpened() const
{
    return videoCapture_.isOpened();
}

bool VideoFileSource::close()
{
    if(videoCapture_.isOpened())
    {
    videoCapture_.release();
    return true;
    }
    return false;
}
#include "detector/HogPeopleDetector.hpp"

HogPeopleDetector::HogPeopleDetector()
{
    // 使用 OpenCV 自带的行人检测模型
    hog_.setSVMDetector(
        cv::HOGDescriptor::getDefaultPeopleDetector()
    );
}

std::vector<cv::Rect>
HogPeopleDetector::detect(const cv::Mat& frame)
{
    std::vector<cv::Rect> people;

    if(frame.empty())
    {
        return people;
    }

    hog_.detectMultiScale(
        frame,
        people,
        0.0,
        cv::Size(8,8),
        cv::Size(32,32),
        1.05,
        2.0,
        false
    );

    return people;
}
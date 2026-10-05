#pragma once

#include "ports/IPeopleDetector.hpp"
#include <opencv2/xobjdetect.hpp>

class HogPeopleDetector : public IPeopleDetector
{
    public:
        HogPeopleDetector();

        // 返回检测到的所有人的矩形框
        std::vector<cv::Rect> detect(const cv::Mat& frame) override;

    private:
        // OpenCV 的 HOG 检测器对象
        cv::HOGDescriptor hog_;
};
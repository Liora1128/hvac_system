#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

class IPeopleDetector
{
    public:
        virtual ~IPeopleDetector() = default;

        virtual std::vector<cv::Rect> detect(const cv::Mat& frame) = 0;
};
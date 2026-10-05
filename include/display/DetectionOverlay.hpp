#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

class DetectionOverlay
{
    public:
        // 在给定的帧上绘制检测到的人的矩形框
        void drawDetections(cv::Mat& frame, const std::vector<cv::Rect>& detections) const;
};
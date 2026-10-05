#pragma once

#include "ports/IPeopleDetector.hpp"

#include <opencv2/dnn.hpp>
#include <string>

class YoloPeopleDetector : public IPeopleDetector
{
    public:
        explicit YoloPeopleDetector(
        const std::string& modelPath
    );

    std::vector<cv::Rect> detect(const cv::Mat& frame) override;

    private:
        cv::dnn::Net net_;

        // yolo 输入尺寸
        int inputWidth_ = 640;
        int inputHeight_ = 640;

        // 置信度阈值
        float confidenceThreshold_ = 0.6f;

        // NMS 阈值
        float nmsThreshold_ = 0.5f;


};
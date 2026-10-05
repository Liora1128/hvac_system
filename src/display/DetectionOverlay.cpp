#include "display/DetectionOverlay.hpp"

#include <opencv2/imgproc.hpp>
#include <string>

void DetectionOverlay::drawDetections(cv::Mat& frame, const std::vector<cv::Rect>& detections) const
{
    if(frame.empty())
    {
        return;
    }

    // 绘制每个人的检测框
    for(const cv::Rect& box : detections)
    {
        cv::rectangle(
            frame,
            box,
            cv::Scalar(0, 255, 0),
            2
        );
    }

    // 生成左上角的人数文字
    std::string text =
        "People: " + std::to_string(detections.size());

    // 绘制人数
    cv::putText(
        frame,
        text,
        cv::Point(20, 40),
        cv::FONT_HERSHEY_SIMPLEX,
        1.0,
        cv::Scalar(0, 255, 0),
        2
    );
}
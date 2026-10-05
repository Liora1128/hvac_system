#pragma once

#include <opencv2/opencv.hpp>

#include <string>

// DisplayWindow class encapsulates the functionality of an OpenCV window for displaying frames.

class DisplayWindow
{
    public:
        explicit DisplayWindow(const std::string& windowName, double screenRatio = 0.8);
        bool open(const cv::Mat& frame);
        void show(const cv::Mat& frame);
        int pollKey(int delay = 1);
        bool isOpen() const;
        void close();
    private:
        std::string windowName_;
        double screenRatio_;
        bool created_;
};
#include "display/DisplayWindow.hpp"

#include <opencv2/highgui.hpp>

#include <windows.h>

#include <algorithm>

DisplayWindow::DisplayWindow(const std::string& windowName, double screenRatio)
    : windowName_(windowName), screenRatio_(screenRatio), created_(false)
{
}

bool DisplayWindow::open(const cv::Mat& frame)
{
    if(frame.empty())
    {
        return false;
    }
    
    RECT workArea;
    if(!SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0))
    {
        return false;
    }

    int screenWidth =
        workArea.right - workArea.left;

    int screenHeight =
        workArea.bottom - workArea.top;

    double maxWidth =
        screenWidth * screenRatio_;

    double maxHeight =
        screenHeight * screenRatio_;
    
    // 分别计算宽度和高度允许的缩放比例，
    // 取较小值，避免任何一个方向超过屏幕。
    double scale = std::min(
        maxWidth / static_cast<double>(frame.cols),
        maxHeight / static_cast<double>(frame.rows)
    );

    scale = std::min(scale, 1.0); // 确保不会放大图像

    int windowWidth = static_cast<int>(frame.cols * scale);
    int windowHeight = static_cast<int>(frame.rows * scale);

    cv::namedWindow(windowName_, cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO | cv::WINDOW_GUI_NORMAL);
    cv::resizeWindow(windowName_, windowWidth, windowHeight);

    int windowX = workArea.left + (screenWidth - windowWidth) / 2;
    int windowY = workArea.top + (screenHeight - windowHeight) / 2;

    cv::moveWindow(windowName_, windowX, windowY);

    created_ = true;
    return true;
}

void DisplayWindow::show(const cv::Mat& frame)
{
    if(!created_ || frame.empty())
    {
        return;
    }
    cv::imshow(windowName_, frame);
}

int DisplayWindow::pollKey(int delay)
{
    if(!created_)
    {
        return -1;
    }
    return cv::waitKey(delay);
}

bool DisplayWindow::isOpen() const
{
    if(!created_)
    {
        return false;
    }

    try
    {
        return cv::getWindowProperty(windowName_, cv::WND_PROP_VISIBLE) >= 1;
    }
    catch(const cv::Exception&)
    {
        return false;
    }
}

void DisplayWindow::close()
{
    if(created_)
    {   
        try
        {
            cv::destroyAllWindows();
        }
        catch(const cv::Exception&)
        {
        }
        created_ = false;
    }
}
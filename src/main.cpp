#include <opencv2/highgui.hpp>

#include <windows.h>
#include <algorithm>

#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

#include <fstream>

#include <string>
#include <iostream>
#include <memory>
#include "ports/IFrameSource.hpp"
#include "camera/OpenCvCameraSource.hpp"
#include "video/VideoFileSource.hpp"
#include "display/DisplayWindow.hpp"
#include "factory/FrameSourceFactory.hpp"

#include "detector/HogPeopleDetector.hpp"
#include "display/DetectionOverlay.hpp"
#include "detector/YoloPeopleDetector.hpp"
#include "factory/PeopleDetectorFactory.hpp"


int main()
{
    // 打开配置文件
    std::ifstream configFile("config/config.json");
    if(!configFile.is_open())
    {
        std::cout << "failed to open config file." << std::endl;
        return 1;
    }

    // 解析json
    nlohmann::json config;
    configFile >> config;

    // 创建帧源工厂
    FrameSourceFactory frameSourceFactory;
    // 创建帧源
    std::unique_ptr<IFrameSource> frameSource = frameSourceFactory.create(config);
    if(!frameSource->open())
    {
        
        std::cerr << "Failed to open " << frameSourceFactory.getFrameSourceType() << " source." << std::endl; 
        return -1;
    }

    std::cout << frameSourceFactory.getFrameSourceType() << " opened successfully." << std::endl;

    // 读取第一帧
    cv::Mat frame;
    if(!frameSource->read(frame))
    {
        std::cerr << "Failed to read frame from " << frameSourceFactory.getFrameSourceType() << "." << std::endl;
        return -1;
    }

    // 创建显示窗口
    DisplayWindow displayWindow("Building HVAC Camera", 0.8);
    if(!displayWindow.open(frame))
    {
        std::cerr << "Failed to open display window." << std::endl;
        return -1;
    }

    // 创建检测器工厂
    PeopleDetectorFactory peopleDetectorFactory;
    
    std::unique_ptr<IPeopleDetector> peopleDetector = 
        peopleDetectorFactory.create(config);
    
    // 防止配置错误导致返回 nullptr
    if(!peopleDetector)
    {
        std::cerr
            << "Failed to create people detector."
            << std::endl;

        return -1;
    }

    std::cout
        << peopleDetectorFactory.getDetectorType()
        << " detector created successfully."
        << std::endl;


    // 创建检测器
    DetectionOverlay overlay;

    

    while(true)
    {
        // 检测目标绘制方框 
        std::vector<cv::Rect> people = peopleDetector->detect(frame);
        overlay.drawDetections(frame, people);

        displayWindow.show(frame);

        // 按下 'q' 键退出
        if(displayWindow.pollKey(1) == 'q')
        {
            break;
        }

        if(displayWindow.pollKey(1) == 27) // ESC key
        {
            break;
        }

        if(!displayWindow.isOpen())
        {
            break;
        }

        // 读取下一帧
        if(!frameSource->read(frame))
        {
            std::cerr << "Failed to read frame from " << frameSourceFactory.getFrameSourceType() << "." << std::endl;
            break;
        }
    }

    displayWindow.close();
    frameSource->close();
    return 0;
}
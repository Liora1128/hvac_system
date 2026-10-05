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
    std::string frameSourceType = config["frame_source"].get<std::string>();

    std::unique_ptr<IFrameSource> frameSource;

    if(frameSourceType == "camera")
    {
        // 创建OpenCV摄像头源
        std::cout << "Creating OpenCV camera source." << std::endl;
        int cameraId = config["camera_id"].get<int>();
        frameSource = std::make_unique<OpencvCameraSource>(cameraId);
    }
    else if(frameSourceType == "video")
    {
        // 创建视频文件源
        std::cout << "Creating video file source." << std::endl;
        std::string videoPath = config["video_path"].get<std::string>();
        frameSource = std::make_unique<VideoFileSource>(videoPath);
    }
    else if(frameSourceType == "image")
    {

    }
    else
    {
        std::cerr << "Unsupported frame source type: " << frameSourceType << std::endl;
        return -1;
    }

    if(!frameSource->open())
    {
        
        std::cerr << "Failed to open " << frameSourceType << " source." << std::endl; 
        return -1;
    }

    std::cout << frameSourceType << " opened successfully." << std::endl;

    cv::Mat frame;
    if(!frameSource->read(frame))
    {
        std::cerr << "Failed to read frame from " << frameSourceType << "." << std::endl;
        return -1;
    }

    DisplayWindow displayWindow("Building HVAC Camera", 0.8);
    if(!displayWindow.open(frame))
    {
        std::cerr << "Failed to open display window." << std::endl;
        return -1;
    }

    while(true)
    {
        displayWindow.show(frame);

        // 按下 'q' 键退出
        if(displayWindow.pollKey(1) == 'q')
        {
            break;
        }

        if(!displayWindow.isOpen())
        {
            break;
        }

        if(!frameSource->read(frame))
        {
            std::cerr << "Failed to read frame from " << frameSourceType << "." << std::endl;
            break;
        }
    }

    displayWindow.close();
    frameSource->close();
    return 0;
}
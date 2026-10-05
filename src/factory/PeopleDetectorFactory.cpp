#include "factory/PeopleDetectorFactory.hpp"

#include "detector/HogPeopleDetector.hpp"
#include "detector/YoloPeopleDetector.hpp"

#include <iostream>


const std::string&
PeopleDetectorFactory::getDetectorType() const
{
    return detectorType_;
}


std::unique_ptr<IPeopleDetector>
PeopleDetectorFactory::create(const nlohmann::json& config)
{
    // 从配置文件读取检测器类型
    detectorType_ =
        config["people_detector"].get<std::string>();


    // 使用 HOG 检测器
    if(detectorType_ == "hog")
    {
        return std::make_unique<HogPeopleDetector>();
    }


    // 使用 YOLO 检测器
    else if(detectorType_ == "yolo")
    {
        // 读取 YOLO 模型路径
        std::string modelPath =
            config["yolo_model"].get<std::string>();
        
        try
        {
            return std::make_unique<YoloPeopleDetector>(
                modelPath
            );
        }
        catch(const std::exception& e)
        {
            std::cerr
                << "Error creating YOLO detector: "
                << e.what()
                << std::endl;

            return nullptr;
        }
    }


    // 配置了不支持的检测器
    else
    {
        std::cerr
            << "Unsupported people detector type: "
            << detectorType_
            << std::endl;

        return nullptr;
    }
}
#include "factory/FrameSourceFactory.hpp"

#include "camera/OpenCvCameraSource.hpp"
#include "video/VideoFileSource.hpp"

#include <iostream>
#include <string>


const std::string&
FrameSourceFactory::getFrameSourceType() const
{
    return frameSourceType_;
}

std::unique_ptr<IFrameSource>
FrameSourceFactory::create(const nlohmann::json& config)
{
    frameSourceType_ = config["frame_source"].get<std::string>();

    if(frameSourceType_ == "camera")
    {
        int cameraId = config["camera_id"].get<int>();
        return std::make_unique<OpencvCameraSource>(cameraId);
    }
    else if(frameSourceType_ == "video")
    {
        std::string videoPath = config["video_path"].get<std::string>();
        return std::make_unique<VideoFileSource>(videoPath);
    }
    else
    {
        std::cerr << "Unsupported frame source type: " << frameSourceType_ << std::endl;
        return nullptr;
    }
}
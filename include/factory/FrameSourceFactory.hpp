#pragma once 

#include "ports/IFrameSource.hpp"
#include "nlohmann/json.hpp"
#include <memory>


class FrameSourceFactory
{
    public:
        std::unique_ptr<IFrameSource>  create(const nlohmann::json& config);
        const std::string& getFrameSourceType() const;
    private:
        std::string frameSourceType_;
};



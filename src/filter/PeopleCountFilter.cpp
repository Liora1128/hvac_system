#include "filter/PeopleCountFilter.hpp"

#include <map>


PeopleCountFilter::PeopleCountFilter(std::size_t windowSize)
    : windowSize_(windowSize)
{
}

int PeopleCountFilter::update(int currentCount)
{
    // 添加当前帧人数到历史记录
    history_.push_back(currentCount);

    // 如果历史记录超过窗口大小，移除最旧的记录
    if(history_.size() > windowSize_)
    {
        history_.pop_front();
    }

    // 计算历史记录中出现次数最多的人数
    // 找出队列里每个数出现的次数
    std::map<int, int> freauency;
    for(int count : history_)
    {
        freauency[count]++;
    }


    // 默认当前帧人数为稳定人数
    int stableCount = currentCount;
    int maxFrequency = freauency[currentCount];

    // 找出出现次数最多的人数
    for(const auto& pair : freauency)
    {
        if(pair.second > maxFrequency)
        {
            maxFrequency = pair.second;
            stableCount = pair.first;
        }
    }

    return stableCount;
}

void PeopleCountFilter::reset()
{   
    // 清除之前保存的所有人数历史
    history_.clear();
}
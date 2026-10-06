#pragma once

#include <deque>
#include <cstddef>


class PeopleCountFilter
{
    public:
        // windowSize：保存最近多少帧的人数
        explicit PeopleCountFilter(std::size_t windowSize = 5);

        // 输入当前帧检测到的人数
        // 返回滤波后的稳定人数
        int update(int currentCount);

        // 清空历史数据
        void reset();

    private:
        // 最近几帧的人数记录
        std::deque<int> history_;

        // 滑动窗口大小，例如 5 帧
        std::size_t windowSize_;
};
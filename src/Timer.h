#pragma once

#include <chrono>
#include <vector>
#include <string>

namespace SpaceWalker {

class Timer
{
public:
    void start();
    void mark(const std::string& name);
    void finish(const std::string& name);
private:
    using vec_time_type = std::invoke_result<decltype(std::chrono::high_resolution_clock::now)>::type;
    std::vector<vec_time_type> _timePoints;
    std::vector<std::string> _names;
};

} // SpaceWalker

#pragma once

#include <chrono>
#include <vector>
#include <string>

class Timer
{
public:
    void start();
    void mark(std::string name);
    void finish(std::string name);
private:
    std::vector<std::chrono::system_clock::time_point> _timePoints;
    std::vector<std::string> _names;
};

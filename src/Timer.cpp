#include "Timer.h"

#include <iostream>

namespace SpaceWalker {

void Timer::start()
{
    _timePoints.push_back(std::chrono::high_resolution_clock::now());
    _names.push_back("Start");
}

void Timer::mark(const std::string& name)
{
    _timePoints.push_back(std::chrono::high_resolution_clock::now());
    _names.push_back(name);
}

void Timer::finish(const std::string& name)
{
    _timePoints.push_back(std::chrono::high_resolution_clock::now());
    _names.push_back(name);

    for (int i = 0; i < _timePoints.size() - 1; i++)
    {
        auto t0 = _timePoints[i];
        auto t1 = _timePoints[i+1];
        auto name = _names[i + 1];

        std::chrono::duration<double> elapsed = t1 - t0;
        std::cout << name << ": " << elapsed.count() * 1000 << " ms | ";
    }
    std::cout << std::endl;
}

} // SpaceWalker
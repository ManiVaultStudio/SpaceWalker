#pragma once

#include <iostream>
#include <string>

class ScopedLogger
{
public:
    ScopedLogger() :
        outputStream(std::cout)
    {
        outputStream.width(80);
        std::cout << std::left;
    }

    ~ScopedLogger()
    {
        std::cout << std::right << "Success" << std::endl;
    }

    void operator<<(const char* str)
    {
        outputStream << str;
    }

private:

    std::ostream& outputStream;
};

#define logger() ScopedLogger logger; logger

class TaskLogger
{
public:
    TaskLogger() :
        outputStream(std::cout)
    {
        outputStream.width(80);
    }

    void newTask(std::string task)
    {
        std::cout << std::left << task;
    }

    void done()
    {
        std::cout << std::right << "Success" << std::endl;
    }

private:
    std::ostream& outputStream;
};

//TaskLogger taskLogger;

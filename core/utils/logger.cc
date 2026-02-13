#include "logger.h"

#include<iostream>
#include<mutex>

static std::mutex log_mutex;

void Logger::log(LogLevel level, const std::string& message){
    std::lock_guard<std::mutex> lock(log_mutex);

    switch(level){
        case LogLevel::INFO:
            std::cout<<"[INFO] ";
            break;
        case LogLevel::WARNING:
            std::cout<<"[WARNING] ";
            break;
        case LogLevel::ERROR:
            std::cout<<"[ERROR] ";
            break;
        case LogLevel::DEBUG:
            std::cout<<"[DEBUG] ";
            break;
    }
    std::cout<<message<<std::endl;
}
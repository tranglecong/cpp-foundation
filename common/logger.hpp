#pragma once
#include <iostream>

#define LOG_INFO(message) std::cout << message << std::endl;
#define LOG_DEBUG(message) std::cout << message << std::endl;
#define LOG_WARNING(message) std::cerr << message << std::endl;
#define LOG_ERROR(message) std::cerr << message << std::endl;
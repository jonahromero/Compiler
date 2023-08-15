#include "Logger.h"
#include <iostream>

void Logger::log(std::string_view view)
{
	std::cout << view;
}

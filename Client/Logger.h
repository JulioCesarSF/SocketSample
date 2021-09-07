#pragma once

#include <iostream>
#include <string>

void Log(std::string msg)
{
	std::cout << msg << std::endl;
}

void WriteLog(std::string msg)
{
	Log(msg);
}
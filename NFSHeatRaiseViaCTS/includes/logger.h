#pragma once

#include <string>

// Debug
extern bool CreateLog;

std::string getTimestamp();
void Log(const std::string& message, bool displayTimestamp = true);
#pragma once

#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

class Logger {
private:
	std::ofstream logFile;

public:
	Logger(const std::string& fileName) {
		logFile.open(fileName, std::ios::app); // Open in append mode
			
		if (!logFile.is_open())
			throw std::runtime_error("Failed to open log file: " + fileName);
	}

	~Logger() // When the logger goes out of scope, ensure the file is closed
	{
		if (logFile.is_open())
			logFile.close();
	}

	std::string getTimeStamp() {
		std::time_t t = std::time(nullptr);
		std::tm tm{};

		localtime_s(&tm, &t);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

		return oss.str();
	}

	void log(const std::string& message) {
		if (logFile.is_open()) {
			logFile << '[' << getTimeStamp() << "] " << message << '\n';
		}
	}
};
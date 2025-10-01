#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>

#include "logger.h"
#include "settings.h"

std::string getTimestamp() {
	std::time_t t = std::time(nullptr);
	std::tm tm{};
	localtime_s(&tm, &t); // Windows-safe version
	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
	return oss.str();
}

void Log(const std::string& message) {
	if (CreateLog) {
		std::ofstream logFile("NFSMWHeatRaiseViaCTS.log", std::ios::app);

		if (logFile.is_open()) {
			logFile << '[' << getTimestamp() << "] ";
			logFile << message << '\n';
		}
	}
}
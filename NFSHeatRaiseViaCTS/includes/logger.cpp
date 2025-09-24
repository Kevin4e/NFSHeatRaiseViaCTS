#include <Windows.h>

#include <fstream>
#include <ctime>

#include "logger.h"

// Debug
bool CreateLog{};

std::string getTimestamp() {
	time_t timestamp;
	time(&timestamp);

	std::string s = ctime(&timestamp);
	s.pop_back(); // Remove the newline character added by ctime

	return s;
}

void Log(const std::string& message, bool displayTimestamp) {
	if (CreateLog) {
		std::ofstream logFile("NFSMWHeatRaiseViaCTS.log", std::ios::app);

		if (!logFile.is_open()) {
			MessageBoxA(nullptr, "Cannot open log file", "Log Error", MB_OK);
			return;
		}

		if (logFile.is_open()) {
			if (displayTimestamp)
				logFile << '[' << getTimestamp() << "] ";
				
			logFile << message << '\n';
			logFile.close();
		}
	}
}
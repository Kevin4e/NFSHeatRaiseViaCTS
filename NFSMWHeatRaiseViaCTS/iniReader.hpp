#pragma once

// Personal INI file reader class

#include <cstdint>
#include <fstream>
#include <string>

class IniReader {
private:
	std::ifstream file;
	std::string line;

	inline void trim(std::string& s) {
		// Trim from start
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));

		// Trim from end
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
	}

	bool findKey(const std::string& key) {
		file.clear();                 // clear EOF flag
		file.seekg(0, std::ios::beg); // rewind to beginning

		while (std::getline(file, line)) {
			size_t pos = line.find('=');

			if (pos == std::string::npos) continue; // skip lines without '='
			
			std::string currentKey = line.substr(0, pos);
			trim(currentKey);

			if (currentKey != key) continue; // not the exact key, continue searching

			line = line.substr(pos + 1);
			trim(line);

			// remove inline comments
			size_t commentPos = line.find("//");
			if (commentPos != std::string::npos) {
				line = line.substr(0, commentPos);
				trim(line);
			}

			return true;
		}

		return false;
	}

public:
	IniReader(const std::string& filename) {
		file.open(filename);
	}

	// Read a signed 8-bit integer from the INI file
	int8_t readInt8(const std::string& key, int8_t defaultValue) {
		if (findKey(key)) {
			try {
				return static_cast<int8_t>(std::stoi(line));
			}
			catch (...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	// Read an unsigned 8-bit integer from the INI file
	uint8_t readUInt8(const std::string& key, uint8_t defaultValue) {
		if (findKey(key)) {
			try {
				return static_cast<uint8_t>(std::stoul(line));
			}
			catch (...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	// Read a signed 16-bit integer from the INI file
	int16_t readInt16(const std::string& key, int16_t defaultValue) {
		if (findKey(key)) {
			try {
				return static_cast<int16_t>(std::stoi(line));
			}
			catch (...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	// Read an unsigned 16-bit integer from the INI file
	uint16_t readUInt16(const std::string& key, uint16_t defaultValue) {
		if (findKey(key)) {
			try {
				return static_cast<uint16_t>(std::stoul(line));
			}
			catch (...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	// Read a signed 32-bit integer from the INI file
	int32_t readInt32(const std::string& key, int32_t defaultValue) {
		if (findKey(key)) {
			try {
				return std::stoi(line);
			}
			catch (...) {
				return defaultValue;
			}
		}
		
		return defaultValue;
	}

	// Read an unsigned 32-bit integer from the INI file
	uint32_t readUInt32(const std::string& key, uint32_t defaultValue) {
		if (findKey(key)) {
			try {
				return std::stoul(line);
			}
			catch (...) {
				return defaultValue;
			}
		}

		return defaultValue;
	}

	// Read a signed 64-bit integer from the INI file
	int64_t readInt64(const std::string& key, int64_t defaultValue) {
		if (findKey(key)) {
			try {
				return std::stoll(line);
			}
			catch (...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	// Read an unsigned 64-bit integer from the INI file
	uint64_t readUInt64(const std::string& key, uint64_t defaultValue) {
		if (findKey(key)) {
			try {
				return std::stoull(line);
			}
			catch (...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	// Read a 32-bit floating point number from the INI file 
	float readFloat(const std::string& key, float defaultValue) {
		if (findKey(key)) {
			try {
				return std::stof(line);
			}
			catch (...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	// Read a double precision floating point number from the INI file (64-bit)
	double readDouble(const std::string& key, double defaultValue) {
		if (findKey(key)) {
			try {
				return std::stod(line);
			}
			catch (...) {
				return defaultValue;
			}
		}
		return defaultValue;
	}

	// Read a single character from the INI file
	char readChar(const std::string& key, char defaultValue) {
		if (findKey(key)) {
			return line.empty() ? defaultValue : line[0];
		}
		return defaultValue;
	}

	// Read a string from the INI file
	std::string readString(const std::string& key, const std::string& defaultValue) {
		if (findKey(key)) {
			return line;
		}

		return defaultValue;
	}
};
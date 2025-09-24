#pragma once

// Personal INI file reader class
// It doesn't support section headers
// Commented out functions are not used in this project.
// They are here for future reference

#include "pch.h"

#include <fstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <sstream>

class IniReader {
private:
	std::string line;
	std::unordered_map<std::string, std::string> data;

	inline void trim(std::string& s) const {
		// Trim from start
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));

		// Trim from end
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
	}

	// Remove inline comments from a line
	inline void removeInlineComment(std::string& s) const {
		for (int i = 0; i < s.length(); ++i) {
			if (s.compare(i, 2, "//") == 0 || s[i] == ';' || s[i] == '#') {
				s = s.substr(0, i);
				break;
			}
		}
	}

	inline bool findKey(const std::string& key) {
		auto it = data.find(key);

		if (it != data.end()) {
			line = it->second;
			return true;
		}

		return false;
	}

public:
	IniReader(const std::string& filename) {
		std::ifstream file(filename);

		while (std::getline(file, line)) {
			size_t pos = line.find('=');

			if (pos == std::string::npos) continue; // skip lines without '='

			removeInlineComment(line);

			std::string keyExtracted = line.substr(0, pos);
			trim(keyExtracted);

			std::string value = line.substr(pos + 1);
			trim(value);

			data[keyExtracted] = value;
		}
	}

	template<typename T>
	T read(const std::string& key, T defaultValue) {
		if (!findKey(key)) return defaultValue;

		try {
			//if constexpr (std::is_same_v<T, int8_t>) return static_cast<int8_t>(std::stoi(line));
			//if constexpr (std::is_same_v<T, uint8_t>) return static_cast<uint8_t>(std::stoi(line));
			//if constexpr (std::is_same_v<T, int16_t>) return static_cast<int16_t>(std::stoi(line));
			//if constexpr (std::is_same_v<T, uint16_t>) return static_cast<uint16_t>(std::stoi(line));
			if constexpr (std::is_same_v<T, int32_t>) return std::stoi(line);
			if constexpr (std::is_same_v<T, uint32_t>) return std::stoul(line);
			//if constexpr (std::is_same_v<T, int64_t>) return std::stoll(line);
			//if constexpr (std::is_same_v<T, uint64_t>) return std::stoull(line);
			if constexpr (std::is_same_v<T, float>) return std::stof(line);
			//if constexpr (std::is_same_v<T, double>) return std::stod(line);
			//if constexpr (std::is_same_v<T, char>) return line.empty() ? defaultValue : line[0];
			if constexpr (std::is_same_v<T, std::string>) {
				std::transform(line.begin(), line.end(), line.begin(), ::tolower);
				return line;
			}
		}
		catch (...) {
			return defaultValue;
		}
	}
};
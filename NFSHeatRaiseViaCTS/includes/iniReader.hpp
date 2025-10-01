#pragma once

// Personal INI file reader class
// It doesn't support section headers

#include <string>
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <sstream>

class IniReader {
private:
	std::unordered_map<std::string, std::string> data;

	inline void trim(std::string& s) const {
		// Trim from start
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));

		// Trim from end
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
	}

	inline void removeInlineComment(std::string& s) const {
		for (int i = 0; i < s.length(); ++i) {
			if (s.compare(i, 2, "//") == 0 || s[i] == ';' || s[i] == '#') {
				s = s.substr(0, i);
				break;
			}
		}
	}

	inline bool findKey(const std::string& key, std::string& outValue) const {
		auto it = data.find(key);

		if (it != data.end()) {
			outValue = it->second;
			return true;
		}

		return false;
	}

public:
	// Extracts all the keys found and their values
	IniReader(const std::string& filename) {
		std::ifstream file(filename);
		std::string line;

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
	T read(const std::string& key, T defaultValue, bool toLowerString = true) const {
		std::string outValue;
		if (!findKey(key, outValue)) return defaultValue;

		try {
			if constexpr (std::is_same_v<T, int8_t>) return static_cast<int8_t>(std::stoi(outValue));
			if constexpr (std::is_same_v<T, uint8_t>) return static_cast<uint8_t>(std::stoi(outValue));
			if constexpr (std::is_same_v<T, int16_t>) return static_cast<int16_t>(std::stoi(outValue));
			if constexpr (std::is_same_v<T, uint16_t>) return static_cast<uint16_t>(std::stoi(outValue));
			if constexpr (std::is_same_v<T, int32_t>) return std::stoi(outValue);
			if constexpr (std::is_same_v<T, uint32_t>) return std::stoul(outValue);
			if constexpr (std::is_same_v<T, int64_t>) return std::stoll(outValue);
			if constexpr (std::is_same_v<T, uint64_t>) return std::stoull(outValue);
			if constexpr (std::is_same_v<T, float>) return std::stof(outValue);
			if constexpr (std::is_same_v<T, double>) return std::stod(outValue);
			if constexpr (std::is_same_v<T, char>) return outValue.empty() ? defaultValue : outValue[0];
			if constexpr (std::is_same_v<T, std::string>) {
				if (toLowerString)
					std::transform(outValue.begin(), outValue.end(), outValue.begin(), [](unsigned char c) { return std::tolower(c); });

				return outValue;
			}
		}
		catch (...) {
			return defaultValue;
		}
	}
};
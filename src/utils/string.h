/*
 * string.h
 *
 *  Created on: Mar 12, 2014
 *      Author: pablocm
 */

#ifndef STRING_H_
#define STRING_H_

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

/*
 * MODIFY BY REFERENCE
 */

// trim from start
static inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(),
			std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
	s.erase(
			std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
			s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}


/*
 * MODIFY BY VALUE
 */

// trim from start
static inline std::string ltrim(const std::string &s2) {
	std::string s = s2;
	s.erase(s.begin(),
			std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string rtrim(const std::string &s2) {
	std::string s = s2;
	s.erase(
			std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
			s.end());
	return s;
}

// trim from both ends
static inline std::string trim(const std::string &s2) {
	std::string s = s2;
	return ltrim(rtrim(s));
}

#endif /* STRING_H_ */

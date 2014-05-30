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
#include <memory>    // for std::unique_ptr
#include <stdarg.h>  // for va_start, etc
#include <cstring>

/*
 * TRIM: MODIFY BY REFERENCE
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
 * TRIM: MODIFY BY VALUE
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

/*
 * Format string, analogous to sprintf().
 * http://stackoverflow.com/a/8098080
 */
static inline std::string string_format(const std::string fmt_str, ...) {
    int final_n, n = ((int)fmt_str.size()) * 2; /* reserve 2 times as much as the length of the fmt_str */
    std::string str;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while(1) {
        formatted.reset(new char[n]); /* wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}

#endif /* STRING_H_ */

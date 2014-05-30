/*
 * datetime.h
 *
 *  Created on: May 29, 2014
 *      Author: pablocm
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include <ctime>
#include <string>

inline std::string TimeToString(std::string formatString, time_t theTime) {
	struct tm *timeinfo;
	timeinfo = localtime(&theTime);

	formatString += '\a'; //force at least one character in the result
	std::string buffer;
	buffer.resize(formatString.size());
	int len = strftime(&buffer[0], buffer.size(), formatString.c_str(), timeinfo);
	while (len == 0) {
		buffer.resize(buffer.size() * 2);
		len = strftime(&buffer[0], buffer.size(), formatString.c_str(), timeinfo);
	}
	buffer.resize(len - 1); //remove that trailing '\a'
	return buffer;
}

inline std::string TimeToString(std::string formatString) {
	return TimeToString(formatString, std::time(nullptr));
}

#endif /* DATETIME_H_ */

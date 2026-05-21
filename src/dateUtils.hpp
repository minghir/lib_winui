#ifndef DATEUTILS_HPP
#define DATEUTILS_HPP

#include <windows.h>
#include <string> // Avem nevoie pentru std::string

int compareSystemTimes(const SYSTEMTIME& a, const SYSTEMTIME& b);
void incrementSystemTime(SYSTEMTIME& st);


std::string convertDateFormat(const std::string& dateStr, const std::string& inputFormat, const std::string& outputFormat);
// Versiunea pentru wstring (Unicode)
std::wstring convertDateFormatW(const std::wstring& dateStr, const std::wstring& inputFormat, const std::wstring& outputFormat);

#endif
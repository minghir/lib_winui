#include "dateUtils.hpp"

#include <stdexcept>
#include <cstdio> // Pentru sprintf_s
#include <algorithm>

// Compară două structuri SYSTEMTIME
// Returnează: -1 dacă a < b, 0 dacă sunt egale, 1 dacă a > b
int compareSystemTimes(const SYSTEMTIME& a, const SYSTEMTIME& b) {
    FILETIME fa, fb;
    SystemTimeToFileTime(&a, &fa);
    SystemTimeToFileTime(&b, &fb);

    if (fa.dwHighDateTime < fb.dwHighDateTime) return -1;
    if (fa.dwHighDateTime > fb.dwHighDateTime) return 1;
    if (fa.dwLowDateTime < fb.dwLowDateTime) return -1;
    if (fa.dwLowDateTime > fb.dwLowDateTime) return 1;
    return 0;
}

// Adaugă o zi la o structură SYSTEMTIME
void incrementSystemTime(SYSTEMTIME& st) {
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);

    // Un număr de 100-nanosecunde într-o zi:
    // 10,000,000 (secunde) * 60 * 60 * 24
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    uli.QuadPart += 864000000000ULL; // Adăugăm o zi

    ft.dwLowDateTime = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;
    FileTimeToSystemTime(&ft, &st);
}







std::string convertDateFormat(const std::string& dateStr, const std::string& inputFormat, const std::string& outputFormat) {
    if (dateStr.length() != inputFormat.length()) return dateStr;

    std::string y = "", m = "", d = "";

    // 1. Parsăm dateStr pe baza inputFormat
    // Căutăm pozițiile yyyy, mm, dd în formatul de intrare
    size_t yPos = inputFormat.find("yyyy");
    size_t mPos = inputFormat.find("mm");
    size_t dPos = inputFormat.find("dd");

    if (yPos != std::string::npos) y = dateStr.substr(yPos, 4);
    if (mPos != std::string::npos) m = dateStr.substr(mPos, 2);
    if (dPos != std::string::npos) d = dateStr.substr(dPos, 2);

    // 2. Construim output-ul pe baza outputFormat
    std::string result = outputFormat;

    // Înlocuim în string-ul de format (folosim un mic artificiu de siguranță)
    auto replaceAll = [&](std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    };

    replaceAll(result, "yyyy", y);
    replaceAll(result, "mm", m);
    replaceAll(result, "dd", d);

    return result;
}

#include "dateUtils.hpp"

std::wstring convertDateFormatW(const std::wstring& dateStr, const std::wstring& inputFormat, const std::wstring& outputFormat) {
    // Verificăm dacă lungimile coincid pentru a evita substr pe index greșit
    if (dateStr.length() != inputFormat.length()) return dateStr;

    std::wstring y = L"", m = L"", d = L"";

    // 1. Parsăm dateStr căutând pozițiile formatelor în inputFormat
    size_t yPos = inputFormat.find(L"yyyy");
    size_t mPos = inputFormat.find(L"mm");
    size_t dPos = inputFormat.find(L"dd");

    if (yPos != std::wstring::npos) y = dateStr.substr(yPos, 4);
    if (mPos != std::wstring::npos) m = dateStr.substr(mPos, 2);
    if (dPos != std::wstring::npos) d = dateStr.substr(dPos, 2);

    // 2. Construim output-ul
    std::wstring result = outputFormat;

    auto replaceAllW = [&](std::wstring& str, const std::wstring& from, const std::wstring& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    };

    if (!y.empty()) replaceAllW(result, L"yyyy", y);
    if (!m.empty()) replaceAllW(result, L"mm", m);
    if (!d.empty()) replaceAllW(result, L"dd", d);

    return result;
}
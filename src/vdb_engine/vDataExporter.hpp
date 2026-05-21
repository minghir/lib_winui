#ifndef VDATAEXPORTER_HPP
#define VDATAEXPORTER_HPP

#include <fstream>
#include <string>
#include <vector>
#include <Windows.h> // Pentru WideCharToMultiByte
#include <filesystem>
class vDataExporter {
public:
    static bool toCSV(const QueryResult& res, const std::wstring& filePath) {
        if (!res.success) return false;

        // Folosim std::ofstream
        std::ofstream file{ std::filesystem::path(filePath), std::ios::trunc };
        if (!file.is_open()) return false;

        // 1. Scriem HEADER-ul (folosind ALIASES pentru a reflecta SELECT-ul)
        // Folosim res.aliases deoarece reflectă exact ce vede utilizatorul
        for (size_t i = 0; i < res.aliases.size(); ++i) {
            // Conversie wide -> string
            std::string colTitle(res.aliases[i].begin(), res.aliases[i].end());

            file << "\"" << colTitle << "\"";
            if (i < res.aliases.size() - 1) file << ",";
        }
        file << "\n";

        // 2. Scriem DATELE
        for (const auto& row : res.data) {
            // Important: row.size() trebuie să corespundă cu res.aliases.size()
            for (size_t i = 0; i < row.size(); ++i) {
                std::wstring wVal = vValueToSimpleString(row[i]);

                // Tratăm caracterele speciale (ghilimelele) în interiorul valorii
                // pentru a nu strica formatul CSV
                std::string sVal;
                for (wchar_t wc : wVal) {
                    if (wc == L'\"') sVal += "\"\""; // Escaping pentru CSV (double quotes)
                    else sVal += (char)wc; // Conversie simplă (pentru caractere non-ASCII e nevoie de UTF-8)
                }

                file << "\"" << sVal << "\"";
                if (i < row.size() - 1) file << ",";
            }
            file << "\n";
        }

        file.flush();
        file.close();
        return true;
    }
private:
    // Helper oficial Windows pentru conversie Wide -> UTF-8
    static std::string toUTF8(const std::wstring& wstr) {
        if (wstr.empty()) return "";
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    static std::wstring vValueToSimpleString(const vValue& val) {
        return std::visit([](auto&& arg) -> std::wstring {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) return L"";
            else if constexpr (std::is_same_v<T, std::wstring>) return arg;
            else if constexpr (std::is_same_v<T, double>) {
                std::wstringstream ss;
                ss << arg;
                return ss.str();
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return arg ? L"True" : L"False";
            }
            else return std::to_wstring(arg);
            }, val);
    }
};

#endif
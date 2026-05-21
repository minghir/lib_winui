#ifndef REPORTUTILS_HPP
#define REPORTUTILS_HPP

#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
namespace fs = std::filesystem;



//std::wstring wstr_read_RTF_file(const std::string& filePath);

//void convertRTFtoPDF(const std::string& rtfFile, const std::string& pdfFile);
//void wconvertRTFtoPDF(const std::string& rtfFile, const std::string& pdfFile);
//void initLibreOffice();
//bool copyFile(const std::string& source, const std::string& destination);
//bool removeFile(const std::string& source);
//void create_dir_if_missing(const std::string& path);

bool tdocsRTFtoPDF(const std::wstring& rtfFile, const std::wstring& pdfDir);
bool tdocsXHTMLtoPDF(const std::wstring& xhtmlFile, const std::wstring& pdfDir);

bool startConsole();
void wconcat_pdfs(const std::vector<std::wstring>& input_files, const std::wstring& output_file);
void concat_pdfs(const std::vector<std::wstring>& inputFiles, const std::wstring& outputFile);
#endif
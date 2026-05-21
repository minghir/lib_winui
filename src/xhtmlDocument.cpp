#include "xhtmlDocument.hpp"
#include "stringUtils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

std::string xhtmlDocument::getDocumentType() {
    return "xhtml";
}

std::string xhtmlDocument::getDocumentName() {
    return document_name;
}

void xhtmlDocument::print() {
    std::wcout << content << std::endl;
}


bool xhtmlDocument::open(const std::string file_name) {
    std::cout << "Sunt in xhtmlDocument open" << std::endl;
    std::wifstream fisier(file_name); // Deschide fișierul în mod wide
    if (!fisier) {
        std::wcout << "Eroare in xhtmlDocument open" << std::endl;
        std::wcerr << L"Eroare la deschiderea fisierului: " << file_name.c_str() << std::endl;
        return L"";
    }

    std::wstringstream buffer;
    buffer << fisier.rdbuf(); // Citește tot conținutul în buffer
    //std::wcerr << L"Am citi fisierul in buffer" << std::endl;
    content = buffer.str(); // Returnează conținutul ca std::wstring
    return true;
}

//bool newDoc(std::string name) override;


bool xhtmlDocument::save(const std::string save_path) {
   
    std::string filePath;
    if (!save_path.empty() && save_path.back() == '\\') {
        filePath = save_path + document_name + ".xhtml";
    }
    else {
        filePath = save_path + "\\" + document_name + ".xhtml";
    }


    std::wofstream outFile(filePath);

    // Verifică dacă fișierul a fost deschis cu succes
    if (!outFile.is_open()) {
        std::cerr << "Eroare: Nu s-a putut deschide fisierul pentru scriere: " << filePath << " !!!!" << std::endl;
        return false;
    }

    
    outFile << content;
    outFile.close();

    return true;

}


bool xhtmlDocument::newDoc(const std::string name) {
    document_name = name;
    content.clear();


    return true;
}


void xhtmlDocument::setContent(const std::wstring text) {
    content += text;
}

std::wstring xhtmlDocument::getContent() {
    return content;
}


void xhtmlDocument::replace(std::wstring str_to_replace, std::wstring str_with_replace) {

    //    std::wcout << "fac replace la:" << str_to_replace << " cu: "<< str_with_replace << std::endl;
    content = rpl_wstr_in_wstr(content, str_to_replace, str_with_replace);


}

void xhtmlDocument::setDocumentName(const std::string name) {
    document_name = name;
}
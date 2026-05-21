
#include "txtDocument.hpp"
#include <iostream>
#include <fstream>
#include <sstream>


std::string txtDocument::getDocumentType(){
    return "txt";
}

std::string txtDocument::getDocumentName(){
    return document_name;
}

void txtDocument::print(){
    std::wcout << content << std::endl;
}

bool txtDocument::open(const std::string file_name){

    std::wifstream fisier(file_name); // Deschide fișierul în mod wide
    if (!fisier) {
        std::wcerr << L"Eroare la deschiderea fișierului: " << file_name.c_str() << std::endl;
        return L"";
    }

    std::wstringstream buffer;
    buffer << fisier.rdbuf(); // Citește tot conținutul în buffer
    content = buffer.str(); // Returnează conținutul ca std::wstring
    return true;
}

bool txtDocument::save(const std::string save_path){

    std::string full_path =  save_path + "\\" + document_name + ".txt";

    std::wofstream outFile(full_path);
    
    if (!outFile.is_open()) {
        std::cerr << "Eroare: Nu s-a putut deschide fișierul pentru scriere: " << full_path << std::endl;
        return false;
    }

    outFile << content;
    outFile.close();
    return true;
}

bool txtDocument::newDoc(const std::string name){
    document_name = name;
    content.clear();
    return true;
}

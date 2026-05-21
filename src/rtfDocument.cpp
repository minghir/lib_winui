
#include "rtfDocument.hpp"
#include "stringUtils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>


std::string rtfDocument::getDocumentType(){
    return "rtf";
}

std::string rtfDocument::getDocumentName(){
    return document_name;
}

void rtfDocument::print(){
    std::wcout << content << std::endl;
}

bool rtfDocument::open(const std::string file_name){
    std::cout << "Sunt in rtfDocument open" << std::endl;
    std::wifstream fisier(file_name); // Deschide fișierul în mod wide
    if (!fisier) {
        std::cout << "Eroare in rtfDocument open" << std::endl;
        std::wcerr << L"Eroare la deschiderea fisierului: " << file_name.c_str() << std::endl;
        return L"";
    }

    std::wstringstream buffer;
    buffer << fisier.rdbuf(); // Citește tot conținutul în buffer
    //std::wcerr << L"Am citi fisierul in buffer" << std::endl;
    content = buffer.str(); // Returnează conținutul ca std::wstring
    return true;
}

bool rtfDocument::save(const std::string save_path){
//    std::cout<<"AAAAAAAAAAAAAAAAA"<<std::endl;
//    std::wcout<<content<<std::endl;

    //std::cout << "SUNT AICI SI document_name = " << document_name << std::endl;
    //std::cout << "SUNT AICI SI save_path = " << save_path << std::endl;

    std::string filePath;
    if (!save_path.empty() && save_path.back() == '\\') {
        filePath = save_path + document_name + ".rtf";
    }
    else {
        filePath = save_path + "\\" + document_name + ".rtf";
    }


    std::wofstream outFile(filePath);
    
    // Verifică dacă fișierul a fost deschis cu succes
    if (!outFile.is_open()) {
        std::cerr << "Eroare: Nu s-a putut deschide fisierul pentru scriere: " << filePath << " !!!!" << std::endl;
        return false;
    }

    // Scrie conținutul lui templateFileContent în fișier
//    outFile << "{\\rtf1\\ansi\n" << getFontTable() << page_width << page_height << L"\n" << page_orientation << L"\n" << content << "}";
    outFile << content;
    // Închide fișierul
    outFile.close();

    return true;

}

bool rtfDocument::newDoc(const std::string name){
    document_name = name;
    content.clear();


    return true;
}

void rtfDocument::addParagraf(const std::wstring paragraf){
    content += L"\\pard"+current_style + L" " + paragraf+L"\n\\par\n"; 
}

void rtfDocument::setCurrentStyle(const std::wstring style){
    current_style = style;
}


/*
void rtfDocument::addRow(std::vector<std::wstring> cells) {


    //std::wstring row_def = L"\\trowd";//\\cellx2000\\cellx4000\n";
    std::wstring row_def = L"\\trowd\\trgaph100\n";
    std::wstring row_data = L"\\intbl ";
    int i = 100;
    for (const std::wstring& cell : cells) {
        i += 100;
        std::cout<<i<<std::endl;
//        row_def += L"\\clbrdrt\\brdrs\\brdrw10\\clbrdrl\\brdrs\\brdrw10\\clbrdrr\\brdrs\\brdrw10\\clbrdrb\\brdrs\\brdrw10\\cellx" + to_wstring<int>(i);
        row_def += L"\\cellx" + to_wstring<int>(i);

        row_data += cell + L" \\cell ";
    }
    row_def += L"\n";
    content += row_def + row_data + L"\n\\row\n";


}

void rtfDocument::addTitleRow(std::vector<std::wstring> cells) {
    std::wstring row_def = L"\\trowd\\trgaph100\n";
    std::wstring row_data = L"\\intbl ";
    int cellStart = 0;
    std::wstring cell;
    for (const std::wstring& cel : cells) {
        cell = wstr_trim(cel);
        int charCount = cell.length(); // Numărul de caractere din celulă
        int cellWidth = (charCount * 80) + 300; // Arial 12pt - 110 twips per caracter + 300 twips padding
        std::cout  << charCount << ":" << cellWidth <<std::endl;

        cellStart += cellWidth;
//        row_def += L"\\clbrdrt\\brdrs\\brdrw10\\clbrdrl\\brdrs\\brdrw10\\clbrdrr\\brdrs\\brdrw10\\clbrdrb\\brdrs\\brdrw10";
        row_def += L"\\cellx" + std::to_wstring(cellStart);
        row_data += cell + L" \\cell ";
    }

    row_def += L"\n";
    content += row_def + row_data + L"\n\\row\n";
}
*/

void rtfDocument::addTitleRow(std::vector<std::wstring> cells) {
    std::wstring row_def = L"\\trowd\\trleft0\\trgaph50\n";
    std::wstring row_data = L"\\intbl ";
    int cellStart = 0;
    std::wstring cell;

    const int PAGE_WIDTH = 16840;  // Dimensiunea A4 Landscape în twips
    int totalWidth = 0;
    std::vector<int> cellWidths;

    // Calculează lățimea inițială a fiecărei celule
    for (const std::wstring& cel : cells) {
        cell = wstr_trim(cel);
        int charCount = cell.length();
        int cellWidth = (charCount * 80) + 300; // Calcul inițial
        cellWidths.push_back(cellWidth);
        totalWidth += cellWidth;
    }

    // Scalează celulele dacă tabelul depășește dimensiunea paginii
    if (totalWidth > PAGE_WIDTH) {
        double scaleFactor = (double)PAGE_WIDTH / totalWidth;
        for (int& width : cellWidths) {
            width = static_cast<int>(width * scaleFactor);
        }
    }

    // Construiește tabelul cu celule ajustate
    for (size_t i = 0; i < cells.size(); ++i) {
        cellStart += cellWidths[i];
        row_def += L"\\cellx" + std::to_wstring(cellStart);
        row_data += cells[i] + L" \\cell ";
    }

    row_def += L"\n";
    content += row_def + row_data + L"\n\\row\n";
}


void rtfDocument::addRow(std::vector<std::wstring> cells) {
    std::wstring row_def = L"\\trowd\\trleft0\\trgaph50\n";
    std::wstring row_data = L"\\intbl ";
//    int cellStart = 0;
    std::wstring cell;
    for (const std::wstring& cel : cells) {
        cell = wstr_trim(cel);
  //      int charCount = cell.length(); // Numărul de caractere din celulă
  //      int cellWidth = (charCount * 80) + 300; // Arial 12pt - 110 twips per caracter + 300 twips padding
  //      std::cout  << charCount << ":" << cellWidth <<std::endl;

  //      cellStart += cellWidth;
//        row_def += L"\\clbrdrt\\brdrs\\brdrw10\\clbrdrl\\brdrs\\brdrw10\\clbrdrr\\brdrs\\brdrw10\\clbrdrb\\brdrs\\brdrw10";
    //    row_def += L"\\cellx" + std::to_wstring(cellStart);
        row_data += cell + L" \\cell ";
    }

    row_def += L"\n";
    content += row_def + row_data + L"\n\\row\n";
}




void rtfDocument::setPageType(const std::wstring pg_type){
    if(pg_type == L"A4"){
        page_orientation = L"\\sectd";
        page_height = L"\\paperh16840";
        page_width = L"\\paperw11907";
    }
}


void rtfDocument::setPageOrientation(const std::wstring pg_orient){
    if(pg_orient == L"Landscape"){
        page_orientation = L"\\sectd\\lndscpsxn";
        page_height = L"\\paperh11907";
        page_width = L"\\paperw16840";

    }else if(pg_orient == L"Portrait"){
        page_orientation = L"\\sectd";
        page_height = L"\\paperh16840";
        page_width = L"\\paperw11907";
    }
    
}

void rtfDocument::setContent(const std::wstring text){
    content += text;
}


std::wstring rtfDocument::getFontTable(){
    std::wstring font_tbl = L"{\\fonttbl\n \
    {\\f0\\fnil Arial;}\n \
    {\\f1\\fswiss Times New Roman;}\n \
    {\\f2\\fnil Arial Narrow;}\n \
    }";
return font_tbl;
}

std::wstring rtfDocument::getContent(){
    return content;
}

void rtfDocument::replace(std::wstring str_to_replace, std::wstring str_with_replace) {

//    std::wcout << "fac replace la:" << str_to_replace << " cu: "<< str_with_replace << std::endl;
  content =  rpl_wstr_in_wstr(content,str_to_replace, str_with_replace); 

    
}


void rtfDocument::setDocumentName(const std::string name){
    document_name = name;
}


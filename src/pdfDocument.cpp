
#include "pdfDocument.hpp"
#include "stringUtils.hpp"
#include "ui/ConsoleManager.hpp"

#include "PDFWriter.h"
#include "PDFPage.h"
#include "PDFUsedFont.h"
#include "PageContentContext.h"

#include <iostream>
#include <fstream>
#include <sstream>

std::string pdfDocument::getDocumentType() {
    return "pdf";
}

std::string pdfDocument::getDocumentName() {
    return document_name;
}

void pdfDocument::print() {
    PDFWriter pdfWriter;

    // Start PDF document
    if (pdfWriter.StartPDF("exemplu.pdf", ePDFVersion13) != PDFHummus::eSuccess) {
        std::cerr << "Eroare la inițializarea PDF-ului." << std::endl;
        return;
    }

    // Creează o pagină A4
    PDFPage* page = new PDFPage();
    page->SetMediaBox(PDFRectangle(0, 0, 595, 842)); // dimensiuni A4 în puncte

    // Încarcă fontul
    PDFUsedFont* font = pdfWriter.GetFontForFile("arial.ttf");
    if (!font) {
        std::cerr << "Eroare la încărcarea fontului." << std::endl;
        delete page;
        pdfWriter.EndPDF();
        return;
    }

    // Configurează opțiunile de text
    AbstractContentContext::TextOptions textOptions(
        font,
        14,
        AbstractContentContext::eGray,
        0 // negru
    );

    // Începe contextul de conținut
    PageContentContext* ctx = pdfWriter.StartPageContentContext(page);
    if (!ctx) {
        std::cerr << "Eroare la crearea contextului de pagină." << std::endl;
        delete page;
        pdfWriter.EndPDF();
        return;
    }

    // Scrie textul
    ctx->WriteText(50, 800, "[TEST] Verificare diacritice în consolă : ș ț ă â î", textOptions);

    // Închide contextul și finalizează pagina
    pdfWriter.EndPageContentContext(ctx);
    pdfWriter.WritePageAndRelease(page);

    // Finalizează PDF-ul
    pdfWriter.EndPDF();

    //std::cout << "PDF generat cu succes: output_vtest.pdf" << std::endl;
}



bool pdfDocument::open(const std::string file_name) {

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

bool pdfDocument::save(const std::string save_path) {

    std::string full_path = save_path + "\\" + document_name + ".txt";

    std::wofstream outFile(full_path);

    if (!outFile.is_open()) {
        std::cerr << "Eroare: Nu s-a putut deschide fișierul pentru scriere: " << full_path << std::endl;
        return false;
    }

    outFile << content;
    outFile.close();
    return true;
}

bool pdfDocument::newDoc(const std::string name) {
    document_name = name;
    content.clear();
    return true;
}


void drawCell(AbstractContentContext* ctx, double x, double y, double w, double h) {
    ctx->q(); // salvează starea grafică

    AbstractContentContext::GraphicOptions strokeOptions(
        AbstractContentContext::eStroke,
        AbstractContentContext::eRGB,
        0x000000, // negru
        0.5         // grosimea liniei
    );

    ctx->DrawRectangle(x, y - h, w, h, strokeOptions);

    ctx->Q(); // restaurează starea grafică
}



void pdfDocument::printTable() {
    PDFWriter writer;
    writer.StartPDF("exemplu.pdf", ePDFVersion13);


   


    PDFPage* page = new PDFPage();
    page->SetMediaBox(PDFRectangle(0, 0, 595, 842)); // A4

    PDFUsedFont* font = writer.GetFontForFile("arial.ttf");

    PageContentContext* ctx = writer.StartPageContentContext(page);

    double startX = 50;
    double startY = 750;
    double cellWidth = 150;
    double cellHeight = 30;

    std::vector<std::vector<std::string>> table = {
        {"Nume", "Prenume", "Varsta"},
        {"Popescu", "Ion", "14"},
        {"Ionescu", "Maria", "28"}
    };

    // opțiuni pentru contur (stroke)
    AbstractContentContext::GraphicOptions strokeOptions(
        AbstractContentContext::eStroke,   // doar contur
        AbstractContentContext::eRGB,      // spațiu de culoare RGB
        0x000000,                          // negru
        1                                  // grosimea liniei
    );

    // Scrie textul în celulă
    AbstractContentContext::TextOptions textOptions(
        font,
        12,
        AbstractContentContext::eGray, // text gri/negru
        0
    );


    for (size_t row = 0; row < table.size(); ++row) {
        for (size_t col = 0; col < table[row].size(); ++col) {
            double x = startX + col * cellWidth;
            double y = startY - row * cellHeight;

            // Desenează conturul celulei
            ctx->DrawRectangle(x, y - cellHeight, cellWidth, cellHeight, strokeOptions);

            

            ctx->WriteText(x + 5, y - 20, table[row][col], textOptions);
        }
    }

    ctx->WriteText(70, 800, utf8_encode(L"[TEST] Verificare diacritice în consolă : ș ț ă â î"), textOptions);
   //  ctx->WriteText(70, 800, wstr_to_str(L"[TEST] Verificare diacritice în consolă : ș ț ă â î"), textOptions);
    //ctx->WriteText(50, 800, "[TEST] Verificare diacritice", textOptions);

    writer.EndPageContentContext(ctx);
    writer.WritePageAndRelease(page);
    writer.EndPDF();
}
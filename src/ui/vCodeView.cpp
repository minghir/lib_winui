#include "vCodeView.hpp"
#include "ConsoleManager.hpp" 
#include "../stringUtils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm> // Pentru std::replace (folosit în mod standard)

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator> // Pentru std::istreambuf_iterator
#include <filesystem>
// Presupunând că această funcție este definită și disponibilă
extern std::wstring utf8_to_wstring(const std::string& str);

// Presupunând că str_to_wstr este disponibil pentru logare
// extern std::wstring str_to_wstr(const std::string& s); 

bool vCodeView::loadFromFile(const std::wstring& filePath) {
    ConsoleManager::getInstance().log(L"[vCodeView::loadFromFile] Se încarcă fișierul: " + filePath);

    //std::ifstream ifs(filePath, std::ios::binary);
    std::ifstream ifs{ std::filesystem::path(filePath), std::ios::binary };
    if (!ifs.is_open()) {
        ConsoleManager::getInstance().log(L"[ERROR] Nu s-a putut deschide fișierul: " + filePath);
        return false;
    }

    std::string utf8Content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    if (utf8Content.empty()) {
        if (m_richEdit) m_richEdit->setText(L"");
        return true;
    }

    std::wstring fileContent = utf8_to_wstring(utf8Content);

    // Normalizare \r\n (Perfectă pentru RichEdit/Edit)
    fileContent.erase(std::remove(fileContent.begin(), fileContent.end(), L'\r'), fileContent.end());

    std::wstring normalizedContent;
    normalizedContent.reserve(fileContent.size() + 100);
    for (wchar_t c : fileContent) {
        if (c == L'\n') normalizedContent += L'\r';
        normalizedContent += c;
    }

    // --- MODIFICARE AICI ---
    // În loc de setText(L""), trimitem către editorul intern
    if (m_richEdit) {
        m_richEdit->setText(normalizedContent);
    }
    // -----------------------

    return true;
}

// Metoda pentru a seta modul Read-Only (utilă pentru vizualizare cod)
void vCodeView::setReadOnly(bool readOnly) {
    // Verificăm dacă avem editorul creat și dacă el are un handle valid
    if (m_richEdit && m_richEdit->getHandle()) {
        SendMessage(m_richEdit->getHandle(), EM_SETREADONLY, (WPARAM)readOnly, 0);

        std::wstring status = readOnly ? L"READ-ONLY" : L"EDITABLE";
        ConsoleManager::getInstance().log(L"[vCodeView::setReadOnly] ID: " + str_to_wstr(m_id) + L" setat la: " + status);
    }
}

void vCodeView::setFontSize(int size) {
    m_fontSize = size;

    if (m_richEdit && m_richEdit->getHandle()) {
        // 1. Aplicăm la RichEdit
        // Framework-ul tău probabil are deja o metodă în vRichEdit
        m_richEdit->setFontSize(size);

        // 2. Aplicăm la Gutter (când îl vei activa)
        /*
        if (m_lineGutter) {
            m_lineGutter->updateFont(L"Consolas", size);
        }
        */

        // 3. Forțăm un re-layout pentru că schimbarea fontului poate modifica 
        // lățimea necesară pentru Gutter (ex: de la 99 la 100 linii)
        applyLayout();

        // 4. Re-colorează (uneori RichEdit pierde formatarea la schimbări majore de font)
         //applayColors(); 
    }
}
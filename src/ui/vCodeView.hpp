#pragma once
#include "vPanel.hpp"
#include "vRichEdit.hpp"
#include "Layouts/AnchorLayout.hpp"
#include "vPopupMenu.hpp"
#include <memory>

class vCodeView : public vPanel {
private:
    vRichEdit* m_richEdit = nullptr;
    // vLineGutter* m_lineGutter = nullptr; // Viitorul control pentru numere

    int m_fontSize = 12;
    const int m_gutterWidth = 50;
    bool m_visibleGutter = false;

    std::unique_ptr<vPopupMenu> m_contextMenu; // Obiectul de meniu
    void initContextMenu(); // Metoda pentru setup

    void drawLineNumbers(HDC hdc);
public:
    vCodeView(HINSTANCE hInstance, const std::string& id, int x, int y, int width, int height, EventDispatcher& dispatcher)
        : vPanel(hInstance, id, x, y, width, height, dispatcher)
    {
        //initContextMenu();
        // Nu uita: vPanel va fi părintele pentru RichEdit
    }
    
    void create(HWND parent) override;


    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
    // Proxy methods: redirecționăm apelurile către m_richEdit-ul intern
    void setText(const std::wstring& text) { if (m_richEdit) m_richEdit->setText(text); }
    std::wstring getText() const { return m_richEdit ? m_richEdit->getText() : L""; }

    vRichEdit* getEditor() { return m_richEdit; }

    bool loadFromFile(const std::wstring& filePath);
    void setReadOnly(bool readOnly);

    void setFontSize(int size);
    void redrawGutter();

    void showContextMenu(int x, int y);
};
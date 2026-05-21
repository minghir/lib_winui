#ifndef VWINCOMMAND_HPP
#define VWINCOMMAND_HPP

#include "vWindow.hpp"
#include "vEdit.hpp"

class vCommandDialog : public vWindow {
private:
    vEdit* m_pEdit = nullptr;

public:
    vCommandDialog(HINSTANCE hInst, EventDispatcher& ed)
        : vWindow(hInst, "cmd_modal_window", WindowType::StandardWindow, false, ed) {}

    bool init() {
        // Creăm fereastra (Dialog Style - fără butoane de minimizat/maximizat)
        //if (!this->create(L"VCommandWindowClass", L"Command", 0, 100, 100, 600, 240)) {
        if (!this->create(L"VCommandWindowClass", L"Command", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 600, 240)) {
           
            return false;
        }
        SetWindowPos(this->getHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        this->setLayoutStrategy(std::make_unique<AnchorLayout>());
        // Adăugăm input-ul (vEdit)
        // Folosim un ID specific "cmd_input_field"
        auto edit = std::make_unique<vEdit>(getInstance(), "cmd_input_field", 10, 10, 560, 140, getEventDispatcher(), EditType::CONSOLE_LINE);
        m_pEdit = edit.get();
        //m_pEdit->setFont(L"Consolas", 26);
        m_pEdit->setFont(L"Cascadia Code", 26);


        m_pEdit->setWidthMode(SizeMode::FILL);
        m_pEdit->setHeightMode(SizeMode::FILL);
        m_pEdit->setMargins(0, 0, 0, 0);
        this->addChild("cmd_input_field", std::move(edit));

        // Înregistrăm o acțiune pentru când vEdit termină (Enter)
        // Folosim dispatcher-ul tău pentru a prinde "lost_focus" pe care vEdit îl trimite deja
        getEventDispatcher().registerHandler("lost_focus", "cmd_input_field", [this]() {
            this->onEnterPressed();
            });
        this->applyLayout();
        return true;
    }
   
    void onEnterPressed() {
        HWND hEdit = m_pEdit->getHandle();

        // 1. Identificăm linia curentă
        LRESULT lineIndex = SendMessage(hEdit, EM_LINEFROMCHAR, (WPARAM)-1, 0);
        LRESULT charIndex = SendMessage(hEdit, EM_LINEINDEX, (WPARAM)lineIndex, 0);
        LRESULT lineLength = SendMessage(hEdit, EM_LINELENGTH, (WPARAM)charIndex, 0);

        if (lineLength > 0) {
            // 2. Extragem textul liniei
            std::vector<wchar_t> buffer(lineLength + 1);
            buffer[0] = static_cast<wchar_t>(lineLength + 1);
            SendMessage(hEdit, EM_GETLINE, (WPARAM)lineIndex, (LPARAM)buffer.data());
            buffer[lineLength] = L'\0';
            std::wstring cmdW(buffer.data());

            LRESULT totalLines = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);

            // 3. Verificăm dacă suntem pe ultima linie
            if (lineIndex < totalLines - 1) {
                // Sărim la final
                int textLength = GetWindowTextLength(hEdit);
                SendMessage(hEdit, EM_SETSEL, (WPARAM)textLength, (LPARAM)textLength);

                // Verificăm dacă ultima linie este goală sau are deja conținut
                // Dacă textul nu se termină în \n, adăugăm noi unul
                std::wstring currentText = m_pEdit->getText(); // Folosim metoda ta existentă
                std::wstring redoCmd = L"";

                if (!currentText.empty() && currentText.back() != L'\n') {
                    redoCmd = L"\r\n";
                }

                redoCmd += cmdW;
                SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)redoCmd.c_str());
            }

            // 4. Trimitem comanda spre execuție
            std::string cmdA = wstr_to_str(cmdW);
            getEventDispatcher().dispatch("command_executed", "cmd_modal_window", cmdA);

            // 5. După execuție, forțăm un singur rând nou pentru a pregăti prompt-ul următor
            int finalPos = GetWindowTextLength(hEdit);
            SendMessage(hEdit, EM_SETSEL, (WPARAM)finalPos, (LPARAM)finalPos);
            //SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)L"\r\n");

            if (cmdA != "brow" && cmdA != "browse") {
                SendMessage(hEdit, WM_VSCROLL, SB_BOTTOM, 0);
                SetFocus(hEdit);
            }
        }

        SendMessage(hEdit, WM_VSCROLL, SB_BOTTOM, 0);
        //SetFocus(hEdit);
        if (GetForegroundWindow() == this->getHandle()) {
            SendMessage(hEdit, WM_VSCROLL, SB_BOTTOM, 0);
            SetFocus(hEdit);
        }
    }   
};

#endif
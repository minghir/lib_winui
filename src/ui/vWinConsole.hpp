#ifndef VEINCONSOLE_HPP
#define VEINCONSOLE_HPP

#include "vCanvas.hpp"
#include "ConsoleManager.hpp"
#include "EventDispatcher.hpp"
#include "FontManager.hpp"

class vWinConsole : public vCanvas, public ILogOutput {
private:
    struct LogLine {
        std::wstring text;
        LogLevel level;
    };
    std::vector<LogLine> m_logHistory;
    bool m_isDrawing = false;

public:
    vWinConsole(HINSTANCE hInst, const std::string& id, int x, int y, int w, int h, EventDispatcher& ed)
        : vCanvas(hInst, id, x, y, w, h, ed)
    {
        ConsoleManager::getInstance().addOutput(this);

        //this->m_fontName = L"Consolas";
        //this->m_baseFontSize = 28;
        /*
        setOnDraw([this](HDC hdc, int w, int h) {
            m_isDrawing = true;

            TEXTMETRIC tm;
            GetTextMetrics(hdc, &tm);
            int dynamicLineHeight = tm.tmHeight + tm.tmExternalLeading + 2;

            // 1. Începem desenarea de jos în sus
            // Plecăm de la marginea de jos a canvas-ului minus un rând
            int currentY = h - dynamicLineHeight - 5;

            // 2. Parcurgem istoricul invers (de la cel mai nou log la cel mai vechi)
            // rbegin() ne dă ultimul element adăugat
            for (auto it = m_logHistory.rbegin(); it != m_logHistory.rend(); ++it) {
                const auto& line = *it;

                COLORREF color = RGB(200, 200, 200);
                if (line.level == LogLevel::LOG_ERROR) color = RGB(255, 80, 80);
                else if (line.level == LogLevel::SUCCESS) color = RGB(100, 255, 100);
                else if (line.level == LogLevel::WARNING) color = RGB(255, 255, 100);

                SetTextColor(hdc, color);
                TextOut(hdc, 10, currentY, line.text.c_str(), (int)line.text.length());

                // 3. Urcăm coordonata Y pentru rândul anterior
                currentY -= dynamicLineHeight;

                // 4. Optimizare: dacă am ieșit prin partea de sus a canvas-ului, ne oprim
                if (currentY < -dynamicLineHeight) break;
            }

            m_isDrawing = false;
            });
        */
        
        setOnDraw([this](HDC hdc, int w, int h) {
            m_isDrawing = true;

            int currentDpi = getCurrentDpi();
            if (currentDpi == 0) currentDpi = 96;

            // Calculăm dimensiunea fontului (negative pentru height corect)
            int fontSize = -MulDiv(m_baseFontSize, currentDpi, 96);
            //HFONT hFont = FontManager::getInstance().getFont(m_fontName, fontSize, FW_BOLD, currentDpi);

            HFONT hFont = FontManager::getInstance().getFont(
                m_fontName,
                fontSize,
                m_fontWeight,    // Folosește membrul clasei, nu FW_BOLD hardcodat
                m_fontItalic,    // ADAUGĂ ACESTA
                m_fontUnderline  // ADAUGĂ ACESTA
            );


            if (hFont) {
                HGDIOBJ oldFont = SelectObject(hdc, hFont);
                SetBkMode(hdc, TRANSPARENT);

                TEXTMETRIC tm;
                GetTextMetrics(hdc, &tm);
                int dynamicLineHeight = tm.tmHeight + tm.tmExternalLeading + 2;

                int currentY = h - dynamicLineHeight - 5;

                for (auto it = m_logHistory.rbegin(); it != m_logHistory.rend(); ++it) {
                    const auto& line = *it;

                    // --- SINCRONIZARE CULORI CU CONSOLEMANAGER ---
                    COLORREF textColor = RGB(200, 200, 200); // Alb/Gri (Default)

                    switch (line.level) {
                    case LogLevel::LOG_ERROR:
                        textColor = RGB(255, 50, 50);   // Roșu aprins (Intensity)
                        break;
                    case LogLevel::FATAL_ERROR:
                        textColor = RGB(255, 255, 255); // Text alb pe...
                        // Opțional: poți desena un dreptunghi roșu în spate aici
                        break;
                    case LogLevel::SUCCESS:
                        textColor = RGB(0, 255, 0);     // Verde (FOREGROUND_GREEN)
                        break;
                    case LogLevel::WARNING:
                        textColor = RGB(255, 255, 0);   // Galben (RED + GREEN)
                        break;
                    case LogLevel::DEBUG:
                        textColor = RGB(80, 80, 255);   // Albastru (BLUE + INTENSITY)
                        break;
                    case LogLevel::INFO:
                        textColor = RGB(200, 200, 200); // Albastru deschis
                        break;
                    }

                    SetTextColor(hdc, textColor);
                    TextOutW(hdc, 10, currentY, line.text.c_str(), (int)line.text.length());

                    currentY -= dynamicLineHeight;
                    if (currentY < -dynamicLineHeight) break;
                }

                SelectObject(hdc, oldFont);
            }
            m_isDrawing = false;
            });
            
    }

    // În destructorul ferestrei tale de consolă:
    ~vWinConsole() {
        // Spune-i managerului să te scoată din listă înainte să mori
        ConsoleManager::getInstance().removeExtraOutput(this);
    }

    void writeLog(const std::wstring& message, LogLevel level) override {
        if (m_isDrawing) return;


        if (level == LogLevel::INFO) return;

        // Folosim un stream pentru a sparge mesajul în rânduri separate
        std::wstringstream ss(message);
        std::wstring line;

        while (std::getline(ss, line, L'\n')) {
            // Dacă rândul se termină în \r (pe Windows), îl curățăm
            if (!line.empty() && line.back() == L'\r') {
                line.pop_back();
            }

            m_logHistory.push_back({ line, level });
        }

        // Limităm istoricul (poți mări la 500 pentru tabele mai mari)
        while (m_logHistory.size() > 500) {
            m_logHistory.erase(m_logHistory.begin());
        }

        // Refresh grafic
        if (m_handle) {
            InvalidateRect(m_handle, NULL, TRUE);
        }
    }
};
#endif
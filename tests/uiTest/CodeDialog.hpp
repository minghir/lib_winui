#pragma once

#include "ui\vWindow.hpp"
#include "ui\vCodeView.hpp"
#include "ui\Lexer\CodeLexer.hpp"
#include "ui\vApp.hpp"

#include <string>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

class CodeDialog : public vWindow
{
private:
	std::string m_filePath;
	CodeLexer m_lexer;
    vCodeView* m_codeView;
public:
	CodeDialog(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher, const std::string& filePath)
		: vWindow(hInstance, id, WindowType::StandardWindow, false, dispatcher), m_filePath(filePath)
	{
        std::string syntaxPath = "syntaxes/";

        // Scanăm folderul pentru orice fișier .xml
        try {
            for (const auto& entry : std::filesystem::directory_iterator(syntaxPath)) {
                if (entry.path().extension() == ".xml") {
                    // Metoda ta care folosește XmlCache intern
                    m_lexer.loadLanguageFile(entry.path().string());
                }
            }
        }
        catch (...) {
            LOG_ERROR(L"Nu am putut accesa folderul de sintaxe.");
        }
	}

    void init() {
        auto m_instance = vApp::getAppInstance()->getInstance();

        auto cView = std::make_unique<vCodeView>(m_instance, "c_view", 0, 0, 200, 400, getEventDispatcher());

        // CORECT: Atribuim adresa membrului clasei, nu mai declarăm o variabilă nouă locală!
        this->m_codeView = cView.get();
        //this->m_codeView->init();
        addChild("c_view", std::move(cView));

        m_codeView->setHeightMode(SizeMode::FILL);
        m_codeView->setWidthMode(SizeMode::FILL);

        //m_codeView->loadFromFile(L"D:\\msys2\\home\\victor.minghir\\oli\\examples\\canvas_diamond_wire.oli");
        //m_lexer.setLanguageByFile(L"D:\\msys2\\home\\victor.minghir\\oli\\examples\\canvas_diamond_wire.oli");

        m_codeView->loadFromFile(L"CodeDialog.hpp");
        m_lexer.setLanguageByFile(L"CodeDialog.hpp");

        m_lexer.highlight(m_codeView->getEditor());

        m_codeView->setFontSize(12);
    }

    void applayColors() {
        if (m_codeView) {
            m_lexer.highlight(m_codeView->getEditor());
        }
        else {
            LOG_ERROR(L"applayColors: m_codeView este NULL!");
        }
    }

};


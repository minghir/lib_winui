#ifndef VXMLDIALOG_HPP
#define VXMLDIALOG_HPP

#include "D:\Programming\Aplicatii_C2019\ANC\thirdparty\pugixml-1.15\src\pugixml.hpp"

#include "vWindow.hpp"
#include "FontManager.hpp"
#include "ConsoleManager.hpp"
#include "vMessageDialog.hpp"
#include <string>
#include <Windows.h>
#include <cstdio>

COLORREF HexToColor(const std::string& hexInput);

class vXmlDialog : public vWindow {
protected:
    std::string m_xmlPath;
    std::map<std::string, FontKey> m_xmlFontMap;
    std::map<std::string, std::function<void()>> m_functionRegistry;

    bool m_forceReload = false;


    void parseChildren(pugi::xml_node parentNode, vControl* parentCtrl);

    COLORREF parseColor(const char* hexStr);
    void applyCommonAttributes(pugi::xml_node node, vControl* ctrl);

    void registerFunction(const std::string& name, std::function<void()> func) {
        m_functionRegistry[name] = func;
    }

    

public:

    vXmlDialog(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher, const std::string& xmlPath)
        : vWindow(hInstance, id, WindowType::StandardWindow, false, dispatcher), m_xmlPath(xmlPath)
    {

    }

    void setForceReload(bool force) { m_forceReload = force; }

    virtual void init() {
        registerBaseFunctions();
        // Aici pot fi adăugate și alte apeluri de configurare standard
        loadFromXml();
    }

    virtual void registerBaseFunctions() {
        //registerFunction("onClick", [this]() { this->onHandleOk(); });
        registerFunction("onCancel", [this]() { this->onHandleCancel(); });
        registerFunction("onOK", [this]() { this->onHandleOk(); });
    }



    void loadFromXml();
    
    void setupHandler(vControl* ctrl, const std::string& event, const std::string& funcName) {
        auto it = m_functionRegistry.find(funcName);
        if (it != m_functionRegistry.end()) {

            // 1. Înregistrare standard (fără argumente)
            ctrl->on(event, it->second);

            // 2. Înregistrare pentru evenimente cu argument (vDbGridPicker, ComboBox, etc.)
            // Creăm un lambda care ignoră argumentul string și apelează funcția ta void()
            auto voidFunc = it->second;
            m_dispatcher.registerHandler(event, ctrl->getId(), [voidFunc](const std::string& arg) {
                voidFunc();
                });

            LOG_DEBUG(L"[XML] Handler dublu mapat pentru: " + str_to_wstr(ctrl->getId()));
        }
    }

    virtual void onHandleOk() {
        ConsoleManager::getInstance().log(L"Butonul OK a fost apăsat!");
    }

    virtual void onHandleCancel() {
        ConsoleManager::getInstance().log(L"Butonul Cancel a fost apăsat!");
        this->close();
    }

    void applySpecificAttribute(vControl* ctrl, const std::string& name, const std::string& value);

    bool isFormValid();
};



#endif
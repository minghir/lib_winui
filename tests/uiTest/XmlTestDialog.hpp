#pragma once

#include "ui/vXmlDbDialog.hpp"
#include "ui/vMessageDialog.hpp"

class XmlTestDialog : public vXmlDbDialog {
private :
    bool m_canSave = true;
public:
    XmlTestDialog(HINSTANCE h, const std::string& id, EventDispatcher& d, const std::string& path, dbConnection* db)
        : vXmlDbDialog(h, id, d, path,db)
    {
        // 1. Înregistrăm funcțiile de bază
        registerBaseFunctions();

        // 2. Adăugăm un eveniment nou, specific doar acestei ferestre
        registerFunction("onSaveToDb", [this]() { this->saveData(); });
        registerFunction("onTest", [this]() { this->onTest(); });
        registerFunction("validateAdress", [this]() { this->validateAdress(); });
        registerFunction("testComboBox", [this]() { this->testComboBox(); });
        
        // 3. Încărcăm XML-ul la final, după ce toate funcțiile sunt în registru
        //loadFromXml();
        vXmlDbDialog::init();
    }

    // Suprascriem comportamentul de OK
    void onHandleOk() override {
        

        if (m_canSave) {
            ConsoleManager::getInstance().log(L"UserEditDialog: Salvez datele și apoi închid!");
            saveData();
            this->close();
        }
        else {
            vMessageDialog::Info(L"Datele nu respecta validarile");
        }

        
        
    }

    void saveData() {
        // Logica ta de salvare...
        
            ConsoleManager::getInstance().log(L"UserEditDialog: Salvez datele");
        
    }

    void onTest() {
        ConsoleManager::getInstance().log(L"UserEditDialog: TESTEZ datele");
    }

    void validateAdress() {
        ConsoleManager::getInstance().log(L"UserEditDialog: Validez adresa");

        // Căutăm controlul în ierarhia ferestrei curente
        //vEdit* editAdresa = dynamic_cast<vEdit*>(this->getChild("editAdresa"));
        auto editAdresa =  this->findChild<vEdit>("editAdresa");

        if (editAdresa) {
            std::wstring text = editAdresa->getText();
            LOG_DEBUG(L"Textul pentru validare este: " + text);

            // Aici poți apela manual validarea sau poți verifica textul
            if (editAdresa->isValid()) {

                m_canSave = true;
                LOG_INFO(L"Adresa este validă!");
                
            }
            else {
                m_canSave = false;
                vMessageDialog::Info(L"Adresa NU este validă: " + editAdresa->getValidationError());
            }
        }
        else {
            LOG_ERROR(L"Controlul 'editAdresa' nu a fost găsit!");
        }
    }

    void testComboBox() {
        auto editCombo = this->findChild<vComboBox>("editAdresa2");
        std::wstring text = editCombo->getSelectedText();
        LOG_SUCCESS(L"Ati selectat " + text);
    }
};
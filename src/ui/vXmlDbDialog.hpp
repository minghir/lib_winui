#ifndef VXMLDBDIALOG_HPP
#define VXMLDBDIALOG_HPP

#include "vXmlDialog.hpp"
#include "vDbGridPicker.hpp"
#include "vComboBox.hpp"
#include "vDbComboBox.hpp"
#include "vDatePicker.hpp"
#include "vCheckBox.hpp"
#include "../dbConnection.hpp"
#include "../stringUtils.hpp"



class vXmlDbDialog : public vXmlDialog {
protected:
    dbConnection* m_db;
    DbDialogMode m_mode;
    std::string m_stmName;
    std::wstring m_viewName;
    std::wstring m_tableName;
    std::wstring m_primaryKey;
    std::wstring m_currentId;
    bool m_useSoftDelete = true;

    ///std::map<std::wstring, vControl*> m_dbMapping; // Coloana -> Control GUI
    std::map<vControl*, std::wstring> m_controlToDbMapping;
    std::function<void()> m_onSuccessCallback = nullptr;

    std::map<vControl*, std::wstring> m_originalValues;
   
public:
    // Primim xmlPath și îl trimitem la vXmlDialog
    vXmlDbDialog(HINSTANCE hInstance,
        const std::string& id,
        EventDispatcher& dispatcher,
        const std::string& xmlPath,
        dbConnection* db)
        : vXmlDialog(hInstance, id, dispatcher, xmlPath), // <--- Pasăm xmlPath bazei
        m_db(db),
        m_mode(DbDialogMode::Insert),
        m_stmName(id)
    {
    }

    // Această metodă va fi apelată pentru a configura ce înregistrare procesăm
    void setDbConfig(DbDialogMode mode, const std::wstring& table, const std::wstring& view, const std::wstring& pk, const std::wstring& idValue = L"") {
        m_mode = mode;
        m_tableName = table;
        m_viewName = view;
        m_primaryKey = pk;
        m_currentId = idValue;
    }

    void setOnSuccess(std::function<void()> callback) { m_onSuccessCallback = callback; }

    // Suprascriem init pentru a încărca datele automat dacă suntem pe Update/View
    virtual void init() override {
        // 1. Încarcă XML-ul și creează controalele (vXmlDialog::init apelează loadFromXml)
        vXmlDialog::init();

        LOG_INFO(L"[DB Dialog] Inițializare mapări baze de date...");

        m_controlToDbMapping.clear();
        for (auto const& [id, ctrlPtr] : m_children) {
            mapDbControlsRecursively(ctrlPtr.get());
        }

        // 2. Procesăm doar copiii de nivel 0, restul se rezolvă prin recursivitate internă
        for (auto const& [id, ctrlPtr] : m_children) {
            injectDbRecursively(ctrlPtr.get());
            //mapDbControlsRecursively(ctrlPtr.get());
        }

        // 3. Populăm cu date dacă e cazul
        if (!m_currentId.empty() && (m_mode == DbDialogMode::Update || m_mode == DbDialogMode::View || m_mode == DbDialogMode::Delete)) {
            loadRecordFromDb();
        }
    }

    void setSoftDelete(bool useSoft) {
        m_useSoftDelete = useSoft;
    }

    // Verifică starea curentă a modului de ștergere
    bool isSoftDelete() const {
        return m_useSoftDelete;
    }


   

protected:

    void injectDbRecursively(vControl* ctrl) {
        if (!ctrl) return;

        // 1. Injectare în controlul curent
        if (auto pPicker = dynamic_cast<vDbGridPicker*>(ctrl)) {
            pPicker->setDbConnection(m_db);
        }

        if (auto pBox = dynamic_cast<vDbComboBox*>(ctrl)) {
            pBox->setDbConnection(m_db);
            pBox->populate();
        }

        // 2. Navigare în adâncime DOAR dacă este container
        if (auto container = dynamic_cast<vContainer*>(ctrl)) {
            for (auto const& [id, childPtr] : container->getChildren()) {
                // Verificăm să nu injectăm în părinte (safety check)
                if (childPtr.get() != ctrl) {
                    injectDbRecursively(childPtr.get());
                }
            }
        }
    }

    virtual void registerBaseFunctions() override {
        vXmlDialog::registerBaseFunctions();
        // Înregistrăm funcția de salvare pentru butonul OK din XML
        registerFunction("onOK", [this]() { this->onHandleDbAction(); });
    }

   
    void onHandleDbAction() {};

    


    void mapDbControlsRecursively(vControl* ctrl) {
        if (!ctrl) return;

        // Verificăm dacă are atributul dbColumn (presupun că vControl are o metodă să-l dea)
        // Dacă vControl nu stochează atributele, va trebui să le citești în loadFromXml
        std::wstring colName = ctrl->getAttribute(L"dbColumn");

        // LOG AICI:
        if (ctrl->getType() == ControlType::Label) {
            //LOG_DEBUG(L"Verific Label: " + str_to_wstr(ctrl->getId()) + L" | Col: " + colName);
        }

        if (!colName.empty()) {
            //m_dbMapping[colName] = ctrl;
            m_controlToDbMapping[ctrl] = colName;
        }

        vContainer* container = dynamic_cast<vContainer*>(ctrl);
        if (container) {
            for (auto const& [id, childPtr] : container->getChildren()) {
                mapDbControlsRecursively(childPtr.get());
            }
        }
    }


    
    virtual void loadRecordFromDb() {
        if (!m_db || m_currentId.empty() || m_tableName.empty()) return;

        std::wstring query = L"SELECT * FROM " + m_viewName +
            L" WHERE " + m_primaryKey + L" = '" + m_currentId + L"'";
        
        LOG_DEBUG(query);

        if (m_db->execQuery(query, m_stmName)) {
            if (m_db->fetchNextRow(m_stmName)) {

                // Acum iterăm prin fiecare CONTROL individual
                for (auto const& [control, colName] : m_controlToDbMapping) {
                    if (!control) continue;
                    //LOG_WARNING(L"Procesez control ID: " + str_to_wstr(control->getId()) + L" pentru coloana: " + colName);

                    std::wstring value = trim(m_db->fetchFieldByName(colName, m_stmName));
                    m_originalValues[control] = value;
                    // Folosim setText pe controlul generic (dacă vControl are setText virtual)
                    // sau păstrăm logica ta de cast:
                    if (auto edit = dynamic_cast<vEdit*>(control)) {
                        edit->setText(value);
                        //edit->setAttribute(L"dbValue", value);
                    }
                    else if (auto label = dynamic_cast<vLabel*>(control)) {
                        label->setText(value);
                        //label->invalidate(); // Forțează label-ul să se redeseneze!
                    }
                    else if (auto picker = dynamic_cast<vDbGridPicker*>(control)) {
                        picker->setText(value);
                        picker->setSelectedValue(value);
                    }
                    else if (auto combo = dynamic_cast<vComboBox*>(control)) {
                        combo->setText(value);
                    }
                    else if (auto dpicker = dynamic_cast<vDatePicker*>(control)) {
                        std::wstring cleanDate = value;
                        if (cleanDate.length() > 10) {
                            cleanDate = cleanDate.substr(0, 10);
                        }

                        //LOG_WARNING(L"SETEZ DATA CURATĂ: " + cleanDate);
                        m_originalValues[control] = cleanDate;
                        dpicker->setText(cleanDate);
                    }
                    else if (auto pDbCombo = dynamic_cast<vDbComboBox*>(control)) {
                        // Luăm query-ul de populare (ex: "SELECT id, nume FROM mod_plata")
                        std::wstring populateQuery = pDbCombo->getTargetQuery();

                        //LOG_INFO(L"Forțez populare pentru " + str_to_wstr(control->getId()) + L" cu valoarea: " + value);

                        // Trimitem și query-ul și valoarea pe care o vrem selectată
                        pDbCombo->populate(populateQuery, value);
                    }
                    else if (auto checkbox = dynamic_cast<vCheckBox*>(control)) {
                        // Conversie din string (DB) în bool. 
                        // Verificăm variantele comune: "1", "true", "Y" sau "YES"
                        
                        std::wstring upperVal = to_upper(value);

                        // 2. Verificăm starea booleană
                        bool isChecked = (upperVal == L"1" ||
                            upperVal == L"TRUE" ||
                            upperVal == L"Y" ||
                            upperVal == L"YES" ||
                            upperVal == L"DA");
                        checkbox->setChecked(isChecked);
                        m_originalValues[control] = isChecked ? L"1" : L"0";
                        // Opțional: actualizăm și textul dacă checkbox-ul afișează valoarea brută lângă bifă
                        // checkbox->setText(value); 
                    }
                }
            }
        }
    }


    // Metode pentru generare SQL
    bool doInsert() {
        if (!m_db || m_tableName.empty()) {
            LOG_ERROR(L"[DB Insert] Parametri insuficienți pentru insert!");
            return false;
        }

        // Folosim un map pentru a colecta VALORI UNICE per COLOANĂ
        // Key: Nume Coloană, Value: Valoare de inserat
        std::map<std::wstring, std::wstring> uniqueValues;

        for (auto const& [control, colName] : m_controlToDbMapping) {
            if (!control || colName == m_primaryKey) continue;

            if (dynamic_cast<vLabel*>(control)) {
                continue;
            }
            std::wstring val = L"";

            // Folosim aceeași logică de extragere ca la Update
            if (auto pDbCombo = dynamic_cast<vDbComboBox*>(control)) {
                val = pDbCombo->getSelectedStringValue();
            }
            else if (auto pDPiker = dynamic_cast<vDatePicker*>(control)) {
                val = pDPiker->getDateString();
            }
            else if (auto pDbPiker = dynamic_cast<vDbGridPicker*>(control)) {
                val = pDbPiker->getSelectedValue();
            }
            else if (auto pCheckBox = dynamic_cast<vCheckBox*>(control)) {
                // Salvăm "1" sau "0" pentru a fi consistenți cu restul aplicației
                val = pCheckBox->isChecked() ? L"1" : L"0";
            }
            else {
                val = control->getText();
            }

            // Dacă avem mai multe controale pt aceeași coloană, 
            // ultima valoare găsită va câștiga (evităm eroarea SQL)
            uniqueValues[colName] = val;
        }

        if (uniqueValues.empty()) return false;

        std::wstring columns = L"";
        std::wstring values = L"";
        bool first = true;

        for (auto const& [colName, val] : uniqueValues) {
            if (!first) {
                columns += L", ";
                values += L", ";
            }
            columns += colName;
            // Escapăm ghilimelele simple pentru a preveni erori SQL dacă valoarea conține '
            // O variantă simplă: values += L"'" + val + L"'";
            // Dar e mai bine să verifici dacă val nu e goală sau null (în funcție de DB)
            if (val.empty()) {
                // Dacă valoarea e goală, trimitem NULL pentru a evita erori la tipuri numerice/date
                values += L"NULL";
            }
            else {
                // Altfel, trimitem valoarea între ghilimele
                values += L"'" + val + L"'";
            }
            first = false;
        }

        std::wstring sql = L"INSERT INTO " + m_tableName +
            L" (" + columns + L") VALUES (" + values + L")";

        LOG_INFO(L"[DB Insert] Executare SQL: " + sql);
        
        return m_db->execQuery(sql);

        
    }
    
    bool doDelete() {
        if (!m_db || m_tableName.empty() || m_primaryKey.empty() || m_currentId.empty()) {
            LOG_ERROR(L"[DB Delete] Parametri insuficienți pentru delete!");
            return false;
        }

        std::wstring sql;

        if (m_useSoftDelete) {
            // --- SOFT DELETE ---
            // Doar marcăm rândul ca fiind șters
            sql = L"UPDATE " + m_tableName +
                L" SET deleted = true" +
                L" WHERE " + m_primaryKey + L" = '" + m_currentId + L"'";

            LOG_INFO(L"[DB Soft-Delete] Marcăm înregistrarea ca ștearsă.");
        }
        else {
            // --- HARD DELETE ---
            // Ștergere fizică definitivă din tabel
            sql = L"DELETE FROM " + m_tableName +
                L" WHERE " + m_primaryKey + L" = '" + m_currentId + L"'";

            LOG_INFO(L"[DB Hard-Delete] Ștergem definitiv înregistrarea.");
        }

        LOG_INFO(L"[DB Action] Executare SQL: " + sql);

        return m_db->execQuery(sql);
    }

    

    bool doUpdate() {
        if (!m_db || m_tableName.empty() || m_primaryKey.empty() || m_currentId.empty()) {
            LOG_ERROR(L"[DB Update] Parametri insuficienți!");
            return false;
        }

        std::vector<std::wstring> sets;

        // Pentru log-ul de audit (opțional, dar util pentru viitor)
        std::vector<std::wstring> changedLog;

        for (auto const& [control, colName] : m_controlToDbMapping) {
            if (!control || colName == m_primaryKey) continue;

            std::wstring newValue;

            // 1. Extragem valoarea curentă din control
            if (auto pDbCombo = dynamic_cast<vDbComboBox*>(control)) {
                //newValue = std::to_wstring(pDbCombo->getSelectedValue());
                newValue = pDbCombo->getSelectedStringValue();
            }
            else if (auto pDPiker = dynamic_cast<vDatePicker*>(control)) {
                //newValue = std::to_wstring(pDbCombo->getSelectedValue());
                newValue = pDPiker->getDateString();
            }
            else if (auto pDbPiker = dynamic_cast<vDbGridPicker*>(control)) {
                //newValue = std::to_wstring(pDbCombo->getSelectedValue());
                newValue = pDbPiker->getSelectedValue();
            }
            else if (auto pCheckBox = dynamic_cast<vCheckBox*>(control)) {
                // Convertim starea booleană în string-ul pe care îl așteaptă baza de date
                // Folosim "1" pentru bifat și "0" pentru nebifat
                newValue = pCheckBox->isChecked() ? L"1" : L"0";
            }
            else {
                newValue = control->getText();
            }

            // 2. COMPARAȚIA: Verificăm dacă valoarea s-a schimbat față de m_originalValues
            // Folosim .count() sau .find() pentru a fi siguri că avem o valoare originală
            if (m_originalValues.count(control)) {
                if (m_originalValues[control] == newValue) {
                    continue; // Valorile sunt identice, sărim peste această coloană
                }
            }

            // 3. Dacă am ajuns aici, înseamnă că e "Dirty" (s-a schimbat)
            sets.push_back(colName + L" = '" + newValue + L"'");

            // Putem stoca schimbarea pentru un LOG de audit centralizat
            changedLog.push_back(colName + L": " + m_originalValues[control] + L" -> " + newValue);
        }

        // Dacă nu s-a schimbat nimic, nu executăm query-ul
        if (sets.empty()) {
            LOG_INFO(L"[DB Update] Nicio modificare detectată. Update ignorat.");
            return true;
        }

        // Construim SQL-ul final
        std::wstring sql = L"UPDATE " + m_tableName + L" SET ";
        for (size_t i = 0; i < sets.size(); ++i) {
            sql += sets[i] + (i < sets.size() - 1 ? L", " : L"");
        }
        sql += L" WHERE " + m_primaryKey + L" = '" + m_currentId + L"'";

        LOG_INFO(L"[DB Update] Modificări: ");
        for (const auto& logLine : changedLog) LOG_INFO(L"  - " + logLine);
        LOG_INFO(L"[DB Update] Executare SQL: " + sql);

        return m_db->execQuery(sql);
    }

    void setAllControlsReadOnly(bool readOnly) {
        // Pornim de la copiii de nivel 0 ai dialogului
        for (auto const& [id, ctrlPtr] : m_children) {
            setReadOnlyRecursive(ctrlPtr.get(), readOnly);
        }
    }

    void setReadOnlyRecursive(vControl* ctrl, bool readOnly) {
        if (!ctrl) return;

        // 1. Decidem dacă acest control trebuie DEZACTIVAT sau doar lăsat să treacă evenimentele
        // Containerele NU trebuie dezactivate cu EnableWindow(false) dacă vrem să activăm selectiv copii în interiorul lor
        bool isContainer = (ctrl->getType() == ControlType::Panel ||
            ctrl->getType() == ControlType::Window ||
            ctrl->getType() == ControlType::RadioGroup);

        if (!isContainer) {
            // Dacă e un control de tip INPUT (Edit, Button, etc.), îi setăm starea
            ctrl->setEnabled(!readOnly);
        }
        else {
            // Containerele le lăsăm mereu active (TRUE) pentru a permite randarea și 
            // interacțiunea cu copiii care ar putea fi activați manual (ex: btnCancel)
            EnableWindow(ctrl->getHandle(), TRUE);
        }

        // 2. Mergem recursiv la copii
        for (auto const& childPair : ctrl->getChildren()) {
            setReadOnlyRecursive(childPair.second.get(), readOnly);
        }
    }

    void setDbFieldsEnabled(bool enable) {
        // Pornim recursivitatea de la rădăcina dialogului
        for (auto const& [id, ctrlPtr] : m_children) {
            setDbFieldsEnabledRecursive(ctrlPtr.get(), enable);
        }
    }

    void setDbFieldsEnabledRecursive(vControl* ctrl, bool enable) {
        if (!ctrl) return;

        // 1. Verificăm dacă este un câmp de bază de date
        // Folosim metoda hasAttribute pe care o ai deja în vControl
        if (ctrl->hasAttribute(L"dbColumn")) {
            // Dacă este un Edit, vrei probabil să folosești ReadOnly pentru a permite Copy
            if (auto edit = dynamic_cast<vEdit*>(ctrl)) {
                SendMessage(edit->getHandle(), EM_SETREADONLY, (WPARAM)!enable, 0);
            }
            else {
                // Pentru restul (Combo, Picker, DatePicker), folosim EnableWindow
                ctrl->setEnabled(enable);
            }
        }

        // 2. RECURSIVITATE: Mergem mai departe în adâncime
        // Chiar dacă un container nu are "dbColumn", copiii lui ar putea avea!
        if (auto container = dynamic_cast<vContainer*>(ctrl)) {
            for (auto const& childPair : container->getChildren()) {
                setDbFieldsEnabledRecursive(childPair.second.get(), enable);
            }
        }
    }

};

#endif
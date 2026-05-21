#include "vDbEditDialog.hpp"
#include "D:\Programming\Aplicatii_C2019\ANC\thirdparty\pugixml-1.15\src\pugixml.hpp"

class vDbXmlEditDialog : public vDbEditDialog {
private:
    std::string m_configPath;

public:
    vDbXmlEditDialog(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher,
        dbConnection* db, const std::string& xmlPath, EditMode mode = EditMode::Update)
        : vDbEditDialog(hInstance, id, dispatcher, db, mode), m_configPath(xmlPath) {
    }

    void setupEditControls() override {
        pugi::xml_document doc;
        if (!doc.load_file(m_configPath.c_str())) {
            // LOG_ERROR("Nu s-a putut incarca XML-ul: " + m_configPath);
            return;
        }

        m_DbFormPanel->clearChildren();

        pugi::xml_node formNode = doc.child("vForm");

        // 1. Setări generale din XML (opțional)
        std::string table = formNode.attribute("table").as_string();
        std::string idField = formNode.attribute("primaryKey").as_string();

        if (!table.empty()) setTableToEdit(str_to_wstr(table));
        if (!idField.empty()) setUniqueIdField(idField);

        // 2. Parsăm controalele și le adăugăm în Panel
        if (m_DbFormPanel) {
            for (pugi::xml_node ctrl : formNode.child("controls").children("control")) {

                std::string field = ctrl.attribute("field").as_string();
                std::string label = ctrl.attribute("label").as_string();
                std::string typeStr = ctrl.attribute("type").as_string();

                ControlType type = ControlType::Edit; // Default
                if (typeStr == "combo") type = ControlType::Combobox;
                if (typeStr == "passw") type = ControlType::Edit;
                // adaugă alte tipuri aici

                // Adăugăm direct în structura panelului
                m_DbFormPanel->addField(str_to_wstr(label), field, type);
            }

            // 3. Generăm efectiv controalele (logica de layout discutată anterior)
            m_DbFormPanel->generateFormControls();
        }

        // 4. După ce controalele există, putem face populate
        if (getMode() != EditMode::Insert) {
            populateControls();
        }
    }
};
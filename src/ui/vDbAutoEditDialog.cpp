#include "vDbAutoEditDialog.hpp"
#include "../sql/SqlQueryParser.hpp"
#include "ConsoleManager.hpp"

vDbAutoEditDialog::vDbAutoEditDialog(HINSTANCE hInstance, const std::string& id, EventDispatcher& dispatcher, dbConnection* db)
    : vDbEditDialog(hInstance, id, dispatcher, db, EditMode::Update)
{
    // Constructorul clasei de bază se ocupă de layout-ul general
}

void vDbAutoEditDialog::setupEditControls() {
    if (!m_DbFormPanel) return;
    
    m_DbFormPanel->clearChildren();
    auto columns = m_db->getTableSchema(m_editTable);// m_db->getColumnNames();
        
        for (const auto& col : columns) {

            std::wstring label = col.name;
            m_DbFormPanel->addField(col.name, wstr_to_str(col.name), ControlType::Edit);
        }
    //}
    

    if (m_DbFormPanel->getHandle()) {
        // Această logică ar trebui să fie într-o metodă publică a panelului
        // pe care o numim 'syncUI()' sau 'rebuild()'.
        m_DbFormPanel->rebuildForm();
    }

    if (m_mode == EditMode::Update || m_mode == EditMode::Delete) {
        auto ctrl = m_DbFormPanel->getChild("edit_" + getUniqueIdField());
        if (ctrl) {
            ctrl->setEnabled(false);
        }
        else {
            LOG_ERROR(L"NU AM GASIT ctrl cu id: edit_" + str_to_wstr(getUniqueIdField()));
        }
        populateControls();
    }
}
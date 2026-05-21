#ifndef VDBFORMPANEL_HPP
#define VDBFORMPANEL_HPP

#include "vPanel.hpp"
#include "../dbConnection.hpp"

struct vDbFormPanelControl {
    std::wstring labelText;      // Textul afișat lângă control (ex: "Nume Client:")
    std::string dbField;         // Numele coloanei în DB (ex: "client_name")
    ControlType type;            // Tipul (Edit, Combobox, DatePicker)
    vControl* uiControl = nullptr; // Pointer către controlul WinAPI creat
    bool visible = true;
    bool enabled = true;
};

class vDbFormPanel : public vPanel {
private:
	std::vector< vDbFormPanelControl> m_FormControls;
	//dbConnection* m_db;
public:
    explicit vDbFormPanel(
        HINSTANCE hInstance,
        const std::string& id,
        int x, int y, int width, int height,
        EventDispatcher& dispatcher
        //dbConnection* db
    );

    void addField(const std::wstring& label, const std::string& dbField, ControlType type) {
        m_FormControls.push_back({ label, dbField, type, nullptr });
    }


    void generateFormControls();
    void rebuildForm();


    const std::vector<vDbFormPanelControl>& getFormControls() const {
        return m_FormControls;
    }

    vControl* getControlByField(const std::string& dbField) {
        for (auto& ctrl : m_FormControls) {
            if (ctrl.dbField == dbField) return ctrl.uiControl;
        }
        return nullptr;
    }

    void create(HWND parent) override;

    

    void clearChildren();

    void setAllControlsEnabled(bool enabled) {
        for (auto& ctrl : m_FormControls) {
            if (ctrl.uiControl && ctrl.uiControl->getHandle()) {
                // Actualizăm starea în structură
                ctrl.enabled = enabled;
                // Aplicăm starea în WinAPI
                EnableWindow(ctrl.uiControl->getHandle(), enabled ? TRUE : FALSE);
            }
        }
    }

    
    int calculateTotalContentHeight();
};

#endif
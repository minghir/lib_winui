#ifndef VRADIOGROUP_HPP
#define VRADIOGROUP_HPP

#pragma once
#include "vPanelGroup.hpp"
#include "vRadioButton.hpp"

class vRadioGroup : public vPanelGroup {
public:
    vRadioGroup(HINSTANCE hInstance, const std::string& id, const std::wstring& title,
        int x, int y, int width, int height, EventDispatcher& dispatcher)
        : vPanelGroup(hInstance, id, title, x, y, width, height, dispatcher)
    {
        m_ControlType = ControlType::Panel; // Sau adaugă RadioGroup în enum dacă preferi
    }

    // Suprascriem addChild pentru a ne asigura că orice RadioButton adăugat 
    // face parte din grupul corect
    void addChild(const std::string& id, std::unique_ptr<vControl> ctrl) {
        if (ctrl->getType() == ControlType::RadioButton) {
            vRadioButton* rb = static_cast<vRadioButton*>(ctrl.get());
            // Setăm automat numele grupului ca fiind ID-ul acestui panel
            rb->setGroupName(this->getId());
        }
        vPanelGroup::addChild(id, std::move(ctrl));
    }

    // Metodă utilitară pentru a afla care buton este selectat
    std::string getSelectedId() {
        for (auto& pair : getChildren()) {
            vControl* ctrl = pair.second.get();
            if (ctrl->getType() == ControlType::RadioButton) {
                vRadioButton* rb = static_cast<vRadioButton*>(ctrl);
                if (rb->isChecked()) return rb->getId();
            }
        }
        return "";
    }

    // Metodă pentru a selecta un anumit buton programatic
    void setSelected(const std::string& id) {
        for (auto& pair : getChildren()) {
            vControl* ctrl = pair.second.get();
            if (ctrl->getType() == ControlType::RadioButton) {
                vRadioButton* rb = static_cast<vRadioButton*>(ctrl);
                rb->setChecked(rb->getId() == id);
            }
        }
    }
};

#endif
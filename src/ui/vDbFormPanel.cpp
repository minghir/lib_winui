#include "vDbFormPanel.hpp"
#include "vEdit.hpp"
#include "vLabel.hpp"
#include "vGroupBox.hpp"
#include "vComboBox.hpp"
#include "Layouts\Layouts.hpp"

vDbFormPanel::vDbFormPanel(
    HINSTANCE hInstance,
    const std::string& id,
    int x, int y, int width, int height,
    EventDispatcher& dispatcher
    //dbConnection* db
    //,const std::map<std::wstring, std::wstring>& columnLabels
) : vPanel(hInstance, id, x, y, width, height, dispatcher)
//m_columnLabels(columnLabels),
//m_db(db)
{
    //setBackgroundColor(RGB(224, 224, 0));
    setLayoutStrategy(std::make_unique<FormLayout>());
    ConsoleManager::getInstance().log(L"[vDbFormPanel::Constructor] Apelat pentru ID: " );
}

void vDbFormPanel::create(HWND parent) {
    setScrollBarOn(true);
    vPanel::create(parent); // Creează HWND-ul panelului
    generateFormControls(); // Creează copiii
}


void vDbFormPanel::clearChildren() {
    // 1. Curățăm structura specifică formularului
    // Nu e nevoie să ștergem manual uiControl aici, deoarece 
    // aceștia sunt deținuți de vPanel::m_children (unique_ptr)
    m_FormControls.clear();

    // 2. Apelăm metoda de curățare din vPanel (sau vControl) 
    // care elimină toți copiii din vectorul de unique_ptr.
    // Dacă vPanel nu are o metodă publică, asigură-te că o ai în vControl:
    // m_children.clear(); // Asta va apela destructorii tuturor controalelor UI

    // Dacă ai implementat removeHandlers anterior, ar fi bine să te asiguri 
    // că m_children.clear() declanșează destructorii care fac cleanup în dispatcher.
    vPanel::clearChildren();

    // 3. Forțăm o redesenare dacă panelul este deja creat
    if (getHandle()) {
        InvalidateRect(getHandle(), NULL, TRUE);
    }
}
/*
void vDbFormPanel::generateFormControls() {
    if (!getHandle()) return; // Nu putem crea copii dacă nu avem un HWND părinte valid

    int startX = 20;
    int startY = 20;
    int labelWidth = 100;
    int controlWidth = 250;
    int rowHeight = 25;
    int spacing = 15;

    for (size_t i = 0; i < m_FormControls.size(); ++i) {
        auto& ctrlInfo = m_FormControls[i];
        int currentY = startY + (int)i * (rowHeight + spacing);

        // 1. Label
        std::string labelId = "lbl_" + ctrlInfo.dbField;
        auto label = std::make_unique<vLabel>(m_hInstance, labelId, ctrlInfo.labelText,
            startX, currentY + 5, labelWidth, rowHeight, m_dispatcher);
        this->addChild(labelId, std::move(label));

        // 2. Edit
        if (ctrlInfo.type == ControlType::Edit) {
            std::string editId = "edit_" + ctrlInfo.dbField;
            auto edit = std::make_unique<vEdit>(m_hInstance, editId, startX + labelWidth + 10,
                currentY, controlWidth, rowHeight, m_dispatcher,
                EditType::SINGLE_LINE);
            ctrlInfo.uiControl = edit.get();
            //LOG_ERROR(L"ADAUG ID:" + str_to_wstr(editId));
            this->addChild(editId, std::move(edit));
        }
    }

    this->applyLayout(); // Actualizăm pozițiile conform strategiei de layout

    // Forțăm Windows să deseneze noile controale imediat
    InvalidateRect(getHandle(), NULL, TRUE);

    if (m_scrollBarOn) {
        int totalHeight = calculateTotalContentHeight(); // Y-ul ultimului control + înălțimea lui
        SCROLLINFO si = { sizeof(si), SIF_RANGE | SIF_PAGE };
        si.nMin = 0;
        si.nMax = totalHeight;
        si.nPage = getHeight(); // Înălțimea vizibilă
        SetScrollInfo(getHandle(), SB_VERT, &si, TRUE);
    }
}
*/
int calculateRequiredLabelWidth(HWND hParent, const std::vector<vDbFormPanelControl>& controls, HFONT hFont) {
    HDC hdc = GetDC(hParent);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    int maxWidth = 0;

    for (const auto& ctrl : controls) {
        // Facem o copie locală a textului pentru trim
        std::wstring text = ctrl.labelText;

        // Trim la final (Right Trim)
        text.erase(text.find_last_not_of(L" \t\r\n") + 1, std::wstring::npos);
        // Trim la început (Left Trim)
        text.erase(0, text.find_first_not_of(L" \t\r\n"));

        if (text.empty()) continue;

        SIZE size;
        if (GetTextExtentPoint32W(hdc, text.c_str(), (int)text.length(), &size)) {
            if (size.cx > maxWidth) maxWidth = size.cx;
        }
    }

    SelectObject(hdc, hOldFont);
    ReleaseDC(hParent, hdc);

    // Padding minim de siguranță
    return maxWidth + 5;
}





void vDbFormPanel::generateFormControls() {
    if (!getHandle()) return;

    this->setLayoutStrategy(std::make_unique<VerticalStackLayout>());

    // --- PASUL A: Calculăm lățimea exactă ---
    HFONT hFont = (HFONT)SendMessage(getHandle(), WM_GETFONT, 0, 0);
    if (!hFont) hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    int dynamicLabelWidth = calculateRequiredLabelWidth(getHandle(), m_FormControls, hFont);

    // Eliminăm limita de 80. Lăsăm doar o limită maximă să nu ocupe tot ecranul dacă textul e imens
    if (dynamicLabelWidth > 350) dynamicLabelWidth = 350;

    int startX = 5;         // O margine mică de siguranță față de marginea ferestrei
    int horizontalGap = 10;  // Spațiul dintre Label și Edit

    for (size_t i = 0; i < m_FormControls.size(); ++i) {
        auto& ctrlInfo = m_FormControls[i];
        LOG_DEBUG(L"Adaug control:"+ str_to_wstr(ctrlInfo.dbField));
        std::string groupId = "grp_" + ctrlInfo.dbField;
        auto rowGroup = std::make_unique<vGroupBox>(m_hInstance, groupId, L"", 0, 0, 0, 45, m_dispatcher);

        rowGroup->setShowBorder(false);
        rowGroup->setHeightMode(SizeMode::FIXED);
        rowGroup->setWidthMode(SizeMode::FILL);
        rowGroup->setMargins(0, 0, 0, 0);

        vGroupBox* groupPtr = rowGroup.get();
        this->addChild(groupId, std::move(rowGroup));

        // --- 1. Label - începe de la startX ---
        std::string labelId = "lbl_" + ctrlInfo.dbField;
        auto label = std::make_unique<vLabel>(
            m_hInstance, labelId, ctrlInfo.labelText,
            startX, 10, dynamicLabelWidth, 25, m_dispatcher
            );

        // Alinierea la dreapta asigură că textul se termină exact unde începe Edit-ul
        
        label->setTextAlign(TextAlign::RIGHT);
        groupPtr->addChild(labelId, std::move(label));

        // --- 2. Edit - calculat fix după label ---
        if (ctrlInfo.type == ControlType::Edit) {
            std::string editId = "edit_" + ctrlInfo.dbField;
            auto edit = std::make_unique<vEdit>(
                m_hInstance, editId,
                startX + dynamicLabelWidth + horizontalGap, 7,
                250, 25,
                m_dispatcher, EditType::SINGLE_LINE
                );

            ctrlInfo.uiControl = edit.get();
            groupPtr->addChild(editId, std::move(edit));
        }

        if (ctrlInfo.type == ControlType::Combobox) {
            std::string comboId = "edit_" + ctrlInfo.dbField;

            // dropdownHeight (ex: 200) determină cât de mult se extinde lista când dai click
            auto combo = std::make_unique<vComboBox>(
                m_hInstance, comboId,
                startX + dynamicLabelWidth + horizontalGap, 7,
                250, 25, // 25 este înălțimea barei închise
                m_dispatcher, 200 // 200 este înălțimea listei deschise
                );

            // Exemplu de populare hardcodată sau dintr-un query separat
            // combo->addItem(L"Activ");
            // combo->addItem(L"Inactiv");

            ctrlInfo.uiControl = combo.get();
            groupPtr->addChild(comboId, std::move(combo));
        }

        groupPtr->applyLayout();
    }

    this->applyLayout();

    // Scrollbar Logic (rămâne neschimbat)
    if (m_scrollBarOn) {
        int totalHeight = 0;
        auto& children = this->getChildren();
        if (!children.empty()) {
            auto& lastChild = children.back().second;
            totalHeight = lastChild->getY() + lastChild->getHeight() + 20;
        }
        SCROLLINFO si = { sizeof(si), SIF_RANGE | SIF_PAGE | SIF_POS, 0, totalHeight, (UINT)getHeight(), 0 };
        SetScrollInfo(getHandle(), SB_VERT, &si, TRUE);
    }

    InvalidateRect(getHandle(), NULL, TRUE);
}


/*
void vDbFormPanel::generateFormControls() {
    if (!getHandle()) return;

    // Panoul principal (this) trebuie să aibă VerticalStackLayout
    // ca să pună GroupBox-urile unul sub altul automat.
    this->setLayoutStrategy(std::make_unique<VerticalStackLayout>());


    for (size_t i = 0; i < m_FormControls.size(); ++i) {
        auto& ctrlInfo = m_FormControls[i];

        // --- 1. Creăm GroupBox-ul pentru rând ---
        // Îl inițializăm cu lățime FILL (0, dar WidthMode::FILL în layout)
        std::string groupId = "grp_" + ctrlInfo.dbField;
        auto rowGroup = std::make_unique<vGroupBox>(
            m_hInstance, groupId,
            L"", // Titlu gol dacă vrei doar un rând discret, sau ctrlInfo.labelText
            0, 0, 0, 45, // Înălțime fixă pentru rând, lățime decisă de layout
            m_dispatcher
            );

        // Opțional: ascundem bordura dacă vrem să arate ca un rând simplu
        rowGroup->setShowBorder(false);
        rowGroup->setHeightMode(SizeMode::FIXED);
        rowGroup->setWidthMode(SizeMode::FILL);
       // rowGroup->setMargin(0,0,0, 5); // Spațiu între rânduri

        vGroupBox* groupPtr = rowGroup.get();
        this->addChild(groupId, std::move(rowGroup));

        // Creăm HWND-ul GroupBox-ului (părintele controalelor de editare)
       // groupPtr->create(getHandle());

        // --- 2. Adăugăm Label în interiorul GroupBox-ului ---
        std::string labelId = "lbl_" + ctrlInfo.dbField;
        auto label = std::make_unique<vLabel>(
            m_hInstance, labelId, ctrlInfo.labelText,
            10, 10, 120, 25, m_dispatcher
            );
        label->setRightAlign(true);
        groupPtr->addChild(labelId, std::move(label));

        // --- 3. Adăugăm Edit în interiorul GroupBox-ului ---
        if (ctrlInfo.type == ControlType::Edit) {
            std::string editId = "edit_" + ctrlInfo.dbField;
            auto edit = std::make_unique<vEdit>(
                m_hInstance, editId,
                140, 7, 250, 25, // X-ul după label
                m_dispatcher, EditType::SINGLE_LINE
                );

            ctrlInfo.uiControl = edit.get();
            groupPtr->addChild(editId, std::move(edit));
        }

        // Forțăm GroupBox-ul să își așeze copiii interni (Label și Edit)

        groupPtr->applyLayout();
    }

    // Așezăm toate GroupBox-urile pe verticală
    this->applyLayout();

    // 2. Calculăm înălțimea totală REALA
    int totalHeight = 0;
    auto& children = this->getChildren();
    if (!children.empty()) {
        // Luăm ultimul copil (ultimul GroupBox)
        auto& lastChild = children.back().second;
        // Înălțimea totală este Y-ul ultimului element + înălțimea lui + o mică margine
        totalHeight = lastChild->getY() + lastChild->getHeight() + 20;
    }

    // 3. Scrollbar Logic
    if (m_scrollBarOn) {
        SCROLLINFO si = { 0 };
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
        si.nMin = 0;
        si.nMax = totalHeight;
        si.nPage = getHeight();
        si.nPos = 0;

        // Dacă nMax <= nPage, scrollbar-ul se va dezactiva/ascunde automat
        SetScrollInfo(getHandle(), SB_VERT, &si, TRUE);
    }

    InvalidateRect(getHandle(), NULL, TRUE);
}
*/




void vDbFormPanel::rebuildForm() {
    // 1. Ștergem tot ce există (UI și metadate m_FormControls sunt gestionate de clearChildren)
    // Atenție: Dacă vrei să păstrezi campurile dar doar să le recreezi UI-ul, 
    // va trebui să separi curățarea m_children de m_FormControls.

    vPanel::clearChildren(); // Șterge doar obiectele vControl (HWND-urile)
    generateFormControls();  // Le recreează pe baza m_FormControls actual
}

int vDbFormPanel::calculateTotalContentHeight() {
    int maxHeight = 0;
    int spacing = 20; // Marginea de jos după ultimul control

    for (const auto& field : m_FormControls) {
        if (field.uiControl) {
            // Luăm poziția Y a controlului + înălțimea lui
            int bottom = field.uiControl->getY() + field.uiControl->getHeight();
            if (bottom > maxHeight) {
                maxHeight = bottom;
            }
        }
    }

    return maxHeight + spacing;
}
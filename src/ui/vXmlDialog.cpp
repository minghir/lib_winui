#include "vXmlDialog.hpp"
#include "vLabel.hpp"
#include "vPanel.hpp"
#include "vEdit.hpp"
#include "vButton.hpp"
#include "vSpacer.hpp"
#include "vComboBox.hpp"
#include "vCheckBox.hpp"
#include "vDatePicker.hpp"
#include "vGroupBox.hpp"
#include "vPanelGroup.hpp"
#include "vRadioGroup.hpp"
#include "vDbGridPicker.hpp"
#include "vDbComboBox.hpp"
#include "vApp.hpp"
#include "Layouts/Layouts.hpp"
#include "../XmlCache.hpp"

COLORREF HexToColor(const std::string& hexInput)
{
    std::string hex = hexInput;

    // Eliminăm spațiile albe dacă există din greșeală în XML
    hex.erase(0, hex.find_first_not_of(" \t\r\n"));
    hex.erase(hex.find_last_not_of(" \t\r\n") + 1);

    if (!hex.empty() && hex[0] == '#')
        hex = hex.substr(1);

    // Verificăm lungimea (suport pentru format scurt FFF sau lung FFFFFF)
    if (hex.length() != 6 && hex.length() != 3)
        return CLR_INVALID; // Returnăm special "invalid" în loc de negru

    try {
        unsigned long value = std::stoul(hex, nullptr, 16);

        if (hex.length() == 3) {
            // Conversie de la RGB scurt (ex: #F00 -> #FF0000)
            int r = ((value >> 8) & 0xF);
            int g = ((value >> 4) & 0xF);
            int b = (value & 0xF);
            return RGB(r | (r << 4), g | (g << 4), b | (b << 4));
        }
        else {
            // Format standard RRGGBB
            int r = (value >> 16) & 0xFF;
            int g = (value >> 8) & 0xFF;
            int b = value & 0xFF;
            return RGB(r, g, b);
        }
    }
    catch (...) {
        return CLR_INVALID;
    }
}

void vXmlDialog::loadFromXml() {

    //auto cachedDoc = XmlCache::getInstance().getXml(m_xmlPath);
    auto cachedDoc = XmlCache::getInstance().getXml(m_xmlPath, m_forceReload);
    if (!cachedDoc) {
        LOG_ERROR(L"Nu s-a putut încărca XML-ul din cache: " + str_to_wstr(m_xmlPath));
        return;
    }
    else {
       
    }

    pugi::xml_node rootNode = cachedDoc->child("Window");
    if (!rootNode) {
        // Fallback în caz că rădăcina are alt nume în fișier
        rootNode = cachedDoc->first_child();
    }

    if (!rootNode) {
        LOG_ERROR(L"[XML] Fisierul XML este gol sau nu are un nod radacina valid!");
        return;
    }

    // pugi::xml_document poate fi tratat ca un node
    //pugi::xml_node root = cachedDoc->child("Window");
    //if (!root) root = cachedDoc->first_child();

    for (auto& child : m_children) {
        m_dispatcher.removeHandlers(child.second->getId());
    }
    this->clearChildren();
    m_dispatcher.removeHandlers(m_id);

    pugi::xml_document doc;
    if (!doc.load_file(m_xmlPath.c_str(), pugi::parse_default, pugi::encoding_utf8)) {
        LOG_ERROR(L"[XML] Nu s-a putut incarca fisierul: " + str_to_wstr(m_xmlPath));
        return;
    }

    // 1. Identificăm nodul rădăcină
    auto root = doc.child("vXmlDialog");
    if (!root) {
        LOG_ERROR(L"[XML] Nodul <vXmlDialog> lipsește!");
        return;
    }
    /*
    std::string xmlName = root.attribute("name").as_string();
    if (!xmlName.empty()) {
        LOG_INFO(L"[XML] Schimb ID fereastră din " + str_to_wstr(m_id) + L" în " + str_to_wstr(xmlName));
        m_id = xmlName; // Actualizăm ID-ul obiectului
    }
    */
    // 2. Încărcăm Fonturile (Căutăm în interiorul lui root, nu doc)
    LOG_INFO(L"[XML] Încarc fonturile...");
    for (auto fNode : root.children("font")) {
        FontKey fk;
        fk.faceName = str_to_wstr(fNode.attribute("faceName").as_string());
        fk.height = fNode.attribute("height").as_int(10);

        std::string wStr = fNode.attribute("weight").as_string();
        if (wStr == "bold") fk.weight = FW_BOLD;
        else if (wStr == "light") fk.weight = FW_LIGHT;
        else fk.weight = FW_NORMAL;

        fk.italic = fNode.attribute("italic").as_bool();
        fk.underline = fNode.attribute("underline").as_bool();
        fk.strikeout = fNode.attribute("strikeout").as_bool();

        std::string fontName = fNode.attribute("name").as_string();
        m_xmlFontMap[fontName] = fk;
        LOG_DEBUG(L"[XML] Font înregistrat: " + str_to_wstr(fontName));
    }

    // 3. Creăm fereastra (HWND)
    std::wstring wTitle = str_to_wstr(root.attribute("title").as_string("Dialog"));
    int x = root.attribute("x").as_int(100);
    int y = root.attribute("y").as_int(100);
    int w = root.attribute("width").as_int(800);
    int h = root.attribute("height").as_int(600);

    this->m_WindowType = WindowType::StandardWindow;
    std::string rawVal = root.attribute("WindowType").as_string();
//    LOG_DEBUG(L"[XML] Valoare bruta WindowType citita: " + str_to_wstr(rawVal));
    if (rawVal == "DialogWindow"){

        this->m_WindowType = WindowType::DialogWindow;
    }
    else if (rawVal == "ToolWindow") {
        this->m_WindowType = WindowType::ToolWindow;

    }
    else if (rawVal == "PopupWindow") {
        this->m_WindowType = WindowType::PopupWindow;
    }

    LOG_INFO(L"[XML] Creez fereastra: " + wTitle);


    DWORD dwStyle =  WS_CLIPCHILDREN;

    if (this->m_WindowType == WindowType::DialogWindow) {
        // Stil tipic de dialog: fără resize (opțional), fără minimize/maximize
        dwStyle |= WS_CAPTION | WS_SYSMENU | WS_POPUP | DS_MODALFRAME;
    }
    else {
        // Stil de fereastră standard (cu de toate)
        dwStyle |= WS_OVERLAPPEDWINDOW;
    }

    HWND hOwner = NULL;
    if (m_parent) {
        hOwner = m_parent->getHandle();
    }
    else {
        // Fallback pe fereastra principală dacă nu s-a setat nimic specific
        hOwner = vApp::getAppInstance()->getMainWindow();
    }

    // Apelăm create-ul din vWindow
    bool success = this->create(L"vXmlDialogClass", wTitle,
        dwStyle,
        x, y, w, h,
        hOwner);

    if (!success) {
        LOG_ERROR(L"[XML] Crearea ferestrei a eșuat!");
        return;
    }
    else {
        this->centerWindow();
    }

    if (success) {
        RECT initialRect;
        GetClientRect(this->getHandle(), &initialRect);
        this->setOriginalClientRect(initialRect); // Acesta setează origWidth/Height

        if (this->m_WindowType == WindowType::DialogWindow) {
            this->centerWindow();
        }

        // 1. Setează Culoarea de Fundal a ferestrei
        std::string bgHex = root.attribute("background").as_string();
        if (!bgHex.empty()) {
            this->setBackgroundColor(HexToColor(bgHex));
        }

        // 2. Setează Culoarea Textului (implicită pentru fereastră)
        std::string textColHex = root.attribute("textColor").as_string();
        if (!textColHex.empty()) {
            this->setTextColor(HexToColor(textColHex));
        }

        // 3. Setează Limitele de dimensiune (MinWidth / MinHeight)
        // Presupunând că ai metodele setMinSize în vWindow
        int minW = root.attribute("minWidth").as_int(0);
        int minH = root.attribute("minHeight").as_int(0);
        if (minW > 0 || minH > 0) {
            // Dacă ai această metodă, o apelăm. Dacă nu, stocăm valorile pentru WM_GETMINMAXINFO
            this->setMinSize(minW, minH);
        }

        // 4. Setează Strategia de Layout pentru fereastra principală
        std::string rootLayout = root.attribute("LayoutStrategy").as_string("AnchorLayout");
        if (rootLayout == "AnchorLayout") {
            this->setLayoutStrategy(std::make_unique<AnchorLayout>());
        }
        else if (rootLayout == "VerticalStackLayout") {
            this->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
        }
        else if (rootLayout == "FlowLayout") {
            this->setLayoutStrategy(std::make_unique<FlowLayout>());
        }
        else if (rootLayout == "FlexStackLayout") {
            this->setLayoutStrategy(std::make_unique<FlexStackLayout>());
        }
        else if (rootLayout == "FormLayout") {
            this->setLayoutStrategy(std::make_unique<FormLayout>());
        }
        else if (rootLayout == "XmlDlgAnchorLayout") {
            this->setLayoutStrategy(std::make_unique<XmlDlgAnchorLayout>());
        }
        else if (rootLayout == "GridLayout") {
            int rows = root.attribute("rows").as_int(2);
            int cols = root.attribute("cols").as_int(2);
            int gap = root.attribute("gap").as_int(5);

            this->setLayoutStrategy(std::make_unique<GridLayout>(rows, cols, gap));
        }
        else if (rootLayout == "HorizontalPercentStackLayout") {
            this->setLayoutStrategy(std::make_unique<HorizontalPercentStackLayout>());
        }
        else if (rootLayout == "VerticalPercentStackLayout") {
            this->setLayoutStrategy(std::make_unique<VerticalPercentStackLayout>());
        }
    }


    // 4. Aplicăm fontul ferestrei (dacă există atributul font pe vXmlDialog)
    std::string rootFontName = root.attribute("font").as_string();
    if (!rootFontName.empty() && m_xmlFontMap.count(rootFontName)) {
        auto& fk = m_xmlFontMap[rootFontName];
        this->setFont(fk.faceName, fk.height, fk.weight, fk.italic, fk.underline);
    }

    // 5. Parsare recursivă a copiilor (Panouri, Label-uri, etc.)
    LOG_INFO(L"[XML] Încep parsarea elementelor UI...");
    parseChildren(root, this);

    // 6. Finalizare Layout
    this->applyLayout();
    //vApp::getAppInstance()->addWindow(winId, std::move(tariWin));
    LOG_INFO(L"[XML] Parsare terminată cu succes!");
}

void vXmlDialog::parseChildren(pugi::xml_node parentNode, vControl* parentCtrl) {
    if (!parentCtrl) {
        LOG_ERROR(L"[XML] Parent control este NULL în parseChildren!");
        return;
    }

    for (pugi::xml_node node : parentNode.children()) {
        if (node.type() != pugi::node_element) continue;

        std::string nodeName = node.name();

        // 1. Handlere de evenimente
        if (nodeName == "handler") {
            std::string eventName = node.attribute("name").as_string();
            std::string functionName = node.attribute("function").as_string();
            if (!eventName.empty() && !functionName.empty()) {
                this->setupHandler(parentCtrl, eventName, functionName);
            }
            continue;
        }

        // 2. Ignorăm nodurile de date interne
        if (nodeName == "text" || nodeName == "default_text" || nodeName == "font" || nodeName == "item") {
            continue;
        }

        std::string type = node.attribute("type").as_string();
        std::string id = node.attribute("name").as_string();
        std::unique_ptr<vControl> newCtrl = nullptr;

        // 3. Fabrica de controale
        if (type == "vPanel" || nodeName == "vContainer" || type == "vGroupBox" || type == "vPanelGroup" || type=="vRadioGroup") {
            vContainer* containerPtr = nullptr;

            if (type == "vRadioGroup") {
                std::wstring title = L"";
                pugi::xml_node titleNode = node.child("text");
                if (titleNode) title = str_to_wstr(titleNode.text().as_string());

                // Creăm RadioGroup (care moștenește vPanelGroup)
                auto rg = std::make_unique<vRadioGroup>(m_hInstance, id, title, 0, 0, 100, 100, m_dispatcher);
                containerPtr = rg.get();
                newCtrl = std::move(rg);
            }
            else if (type == "vGroupBox" ){
                std::wstring title = L"";
                pugi::xml_node titleNode = node.child("text");
                if (titleNode) title = str_to_wstr(titleNode.text().as_string());

                auto gBox = std::make_unique<vGroupBox>(m_hInstance, id, title, 0, 0, 100, 100, m_dispatcher);
                containerPtr = gBox.get();
                newCtrl = std::move(gBox);
            }
            else if (type == "vPanelGroup") {
                // 1. Încercăm să luăm titlul din ATRIBUT (cum e în XML-ul tău)
                std::wstring title = str_to_wstr(node.attribute("title").as_string());

                // 2. Dacă nu e în atribut, încercăm nodul copil <text> (fallback)
                if (title.empty()) {
                    pugi::xml_node titleNode = node.child("text");
                    if (titleNode) title = str_to_wstr(titleNode.text().as_string());
                }

                // 3. Dacă tot e gol, punem un default
                if (title.empty()) title = L"";

                auto pgBox = std::make_unique<vPanelGroup>(m_hInstance, id, title, 0, 0, 100, 100, m_dispatcher);
                containerPtr = pgBox.get();
                newCtrl = std::move(pgBox);
            }
            
            else {
                auto panel = std::make_unique<vPanel>(m_hInstance, id, 0, 0, 100, 100, m_dispatcher);
                containerPtr = panel.get();
                newCtrl = std::move(panel);
            }

            // Setăm strategia de layout pentru container
            if (containerPtr) {
                std::string layoutStr = node.attribute("LayoutStrategy").as_string("VerticalStackLayout");
                if (layoutStr == "VerticalStackLayout") containerPtr->setLayoutStrategy(std::make_unique<VerticalStackLayout>());
                else if (layoutStr == "AnchorLayout") containerPtr->setLayoutStrategy(std::make_unique<AnchorLayout>());
                else if (layoutStr == "GridLayout") {
                    int rows = node.attribute("rows").as_int(2), cols = node.attribute("cols").as_int(2), gap = node.attribute("gap").as_int(5);
                    containerPtr->setLayoutStrategy(std::make_unique<GridLayout>(rows, cols, gap));
                }
                else if (layoutStr == "FlowLayout") containerPtr->setLayoutStrategy(std::make_unique<FlowLayout>());
                else if (layoutStr == "FlexStackLayout") containerPtr->setLayoutStrategy(std::make_unique<FlexStackLayout>());
                else if (layoutStr == "FormLayout") containerPtr->setLayoutStrategy(std::make_unique<FormLayout>());
                else if (layoutStr == "VerticalPercentStackLayout") containerPtr->setLayoutStrategy(std::make_unique<VerticalPercentStackLayout>());
                else if (layoutStr == "HorizontalPercentStackLayout") containerPtr->setLayoutStrategy(std::make_unique<HorizontalPercentStackLayout>());
            }
        }
        else if (type == "vLabel") {
            newCtrl = std::make_unique<vLabel>(m_hInstance, id, m_dispatcher);
        }
        else if (type == "vEdit") {
            newCtrl = std::make_unique<vEdit>(m_hInstance, id, m_dispatcher);
        }
        else if (type == "vButton") {
            newCtrl = std::make_unique<vButton>(m_hInstance, id, L"Btn", 0, 0, 100, 30, m_dispatcher);
        }
        else if (type == "vComboBox") {
            newCtrl = std::make_unique<vComboBox>(m_hInstance, id, m_dispatcher);
        }
        else if (type == "vCheckBox") {
            newCtrl = std::make_unique<vCheckBox>(m_hInstance, id, L"", 0, 0, 100, 25, m_dispatcher);
        }
        else if (type == "vDatePicker") {
            newCtrl = std::make_unique<vDatePicker>(m_hInstance, id, 0, 0, 150, 25, m_dispatcher);
        }
        else if (type == "vDbGridPicker") {
            // Presupunând că constructorul urmează semnătura standard a controalelor tale
            newCtrl = std::make_unique<vDbGridPicker>(m_hInstance, id, 0, 0, 150, 25, m_dispatcher, nullptr);
        }
        else if (type == "vSpacer") {
            newCtrl = std::make_unique<vSpacer>(id, 0, 0, 10, 10, m_dispatcher);
        }
        else if (type == "vRadioButton") {
            // Adăugăm suport pentru butonul radio individual
            newCtrl = std::make_unique<vRadioButton>(m_hInstance, id, L"", 0, 0, 100, 25, m_dispatcher);
        }
        else if (type == "vDbComboBox") {
            // Adăugăm suport pentru butonul radio individual
            newCtrl = std::make_unique<vDbComboBox>(m_hInstance, id,  0, 0, 100, 25, m_dispatcher, nullptr);
        }
        

        // 4. Finalizare și Atribute
        if (newCtrl) {
            applyCommonAttributes(node, newCtrl.get());
            vControl* rawPtr = parentCtrl->addChildWithReturn(id, std::move(newCtrl));

            // Atribute specifice (din tag-ul <attribute>)
            for (pugi::xml_node attrNode : node.children("attribute")) {
                applySpecificAttribute(rawPtr, attrNode.attribute("name").as_string(), attrNode.attribute("value").as_string());
            }

            // Setări specifice după crearea HWND
            if (type == "vLabel" || type == "vButton" || type == "vCheckBox" || type == "vRadioButton") {
                pugi::xml_node txtNode = node.child("text");
                if (txtNode) rawPtr->setText(str_to_wstr(txtNode.text().as_string()));

                if (type == "vCheckBox") {
                    static_cast<vCheckBox*>(rawPtr)->setChecked(node.attribute("checked").as_bool(false));

                    vCheckBox* cb = static_cast<vCheckBox*>(rawPtr);

                    // 1. Citire robustă (verificăm string-ul direct)
                    std::string selAttr = node.attribute("selected").as_string();
                    bool isSelected = (selAttr == "true" || selAttr == "1");

                    // 2. Aplicăm starea
                    cb->setChecked(isSelected);

                    // 3. LOG pentru debug - să vedem în consolă dacă acum e TRUE
                    //LOG_DEBUG(L"Radio " + str_to_wstr(id) + L" selected attribute: " + str_to_wstr(selAttr) +
                    //    L" -> Result: " + (isSelected ? L"TRUE" : L"FALSE"));
                
                }

                if (type == "vRadioButton") {
                    vRadioButton* rb = static_cast<vRadioButton*>(rawPtr);

                    // 1. Citire robustă (verificăm string-ul direct)
                    std::string selAttr = node.attribute("selected").as_string();
                    bool isSelected = (selAttr == "true" || selAttr == "1");

                    // 2. Aplicăm starea
                    rb->setChecked(isSelected);

                    // 3. LOG pentru debug - să vedem în consolă dacă acum e TRUE
                    //LOG_DEBUG(L"Radio " + str_to_wstr(id) + L" selected attribute: " + str_to_wstr(selAttr) +
                    //    L" -> Result: " + (isSelected ? L"TRUE" : L"FALSE"));
                }

            }
            else if (type == "vEdit") {
                vEdit* edit = static_cast<vEdit*>(rawPtr);
                pugi::xml_node defTxtNode = node.child("default_text");
                if (defTxtNode) edit->setText(str_to_wstr(defTxtNode.text().as_string()));

                pugi::xml_node valNode = node.child("validation");
                if (valNode) {
                    edit->setValidation(str_to_wstr(valNode.attribute("pattern").as_string()),
                        str_to_wstr(valNode.attribute("error").as_string()));
                }
            }
            else if (type == "vComboBox") {
                vComboBox* combo = static_cast<vComboBox*>(rawPtr);
                int indexToSelect = -1, currentIndex = 0;
                for (pugi::xml_node itemNode : node.children("item")) {
                    combo->addItem(str_to_wstr(itemNode.text().as_string()));
                    if (itemNode.attribute("selected").as_bool(false)) indexToSelect = currentIndex;
                    currentIndex++;
                }
                if (indexToSelect != -1) combo->setSelectedIndex(indexToSelect);
            }

            else if (type == "vDbGridPicker") {
                vDbGridPicker* edit = static_cast<vDbGridPicker*>(rawPtr);
                pugi::xml_node defTxtNode = node.child("default_text");
                if (defTxtNode) edit->setText(str_to_wstr(defTxtNode.text().as_string()));

                pugi::xml_node valNode = node.child("validation");
                if (valNode) {
                    edit->setValidation(str_to_wstr(valNode.attribute("pattern").as_string()),
                        str_to_wstr(valNode.attribute("error").as_string()));
                }
            }
            else if (type == "vDbComboBox") {
                vDbComboBox* dbCombo = static_cast<vDbComboBox*>(rawPtr);
                int indexToSelect = -1, currentIndex = 0;
                for (pugi::xml_node itemNode : node.children("item")) {
                    dbCombo->addItem(str_to_wstr(itemNode.text().as_string()));
                    if (itemNode.attribute("selected").as_bool(false)) indexToSelect = currentIndex;
                    currentIndex++;
                }
                if (indexToSelect != -1) dbCombo->setSelectedIndex(indexToSelect);
            }
           
            // Recursivitate pentru copiii containerului (inclusiv GroupBox)
            parseChildren(node, rawPtr);
        }
    }
}


Anchor StringToAnchor(const std::string& str) {
    Anchor result = Anchor::NONE;
    if (str.empty()) return Anchor::LEFT | Anchor::TOP; // Default standard

    // Convertim în uppercase pentru a fi case-insensitive
    std::string s = str;
    for (auto& c : s) c = toupper(c);

    if (s.find("LEFT") != std::string::npos)   result = result | Anchor::LEFT;
    if (s.find("RIGHT") != std::string::npos)  result = result | Anchor::RIGHT;
    if (s.find("TOP") != std::string::npos)    result = result | Anchor::TOP;
    if (s.find("BOTTOM") != std::string::npos) result = result | Anchor::BOTTOM;

    // Verificăm noile flag-uri de centrare
    if (s.find("CENTER_H") != std::string::npos) result = result | Anchor::CENTER_H;
    if (s.find("CENTER_V") != std::string::npos) result = result | Anchor::CENTER_V;

    // Dacă scrii doar "CENTER", le activăm pe ambele
    if (s == "CENTER") result = Anchor::CENTER;

    return result;
}


/*
TextAlign StringToAlign(const std::string& str) {
    TextAlign res = TextAlign::LEFT | TextAlign::TOP;
    if (str.find("CENTER") != std::string::npos) res = (res & ~TextAlign::LEFT) | TextAlign::CENTER;
    if (str.find("RIGHT") != std::string::npos)  res = (res & ~TextAlign::LEFT) | TextAlign::RIGHT;
    if (str.find("MIDDLE") != std::string::npos) res = (res & ~TextAlign::TOP) | TextAlign::MIDDLE;
    if (str.find("BOTTOM") != std::string::npos) res = (res & ~TextAlign::TOP) | TextAlign::BOTTOM;
    return res;
}
*/
TextAlign StringToAlign(const std::string& str) {
    // Pornim cu valorile implicite (Top-Left)
    TextAlign res = TextAlign::LEFT | TextAlign::TOP;

    std::string s = str;
    for (auto& c : s) c = toupper(c);

    // --- Orizontal ---
    if (s.find("CENTER") != std::string::npos) {
        res = (res & ~TextAlign::LEFT) | TextAlign::CENTER;
    }
    else if (s.find("RIGHT") != std::string::npos) {
        res = (res & ~TextAlign::LEFT) | TextAlign::RIGHT;
    }

    // --- Vertical ---
    if (s.find("MIDDLE") != std::string::npos || s.find("VCENTER") != std::string::npos) {
        res = (res & ~TextAlign::TOP) | TextAlign::MIDDLE;
    }
    else if (s.find("BOTTOM") != std::string::npos) {
        res = (res & ~TextAlign::TOP) | TextAlign::BOTTOM;
    }

    return res;
}
void vXmlDialog::applyCommonAttributes(pugi::xml_node node, vControl* ctrl) {
    // --- Dimensiuni ---
   // Luăm valorile din XML
    int x = node.attribute("x").as_int(0);
    int y = node.attribute("y").as_int(0);
    int w = node.attribute("width").as_int(100);
    int h = node.attribute("height").as_int(30);

    // IMPORTANT: setRect trebuie să seteze coordonatele de bază (logice)
    // pe care XmlDlgAnchorLayout le va folosi ca referință.
    ctrl->setRect(x, y, w, h);


    // --- Margini (Marginile interioare ale containerului) ---
    int mL = node.attribute("marginLeft").as_int(0);
    int mT = node.attribute("marginTop").as_int(0);
    int mR = node.attribute("marginRight").as_int(0);
    int mB = node.attribute("marginBottom").as_int(0);

    // Dacă controlul este un vContainer (sau derivat ca vGroupBox/vPanel)
    vContainer* container = dynamic_cast<vContainer*>(ctrl);
    if (container) {
        container->setMargins(mL, mT, mR, mB);
    }

    ctrl->setMargins(mL, mT, mR, mB);

    // --- Enabled (Proprietate vControl) ---
    // Implicit este true dacă lipsește atributul
    bool isEnabled = node.attribute("enabled").as_bool(true);
    if(!isEnabled) {
        ctrl->setEnabled(false);
    }

    // --- Culori (Background și Text) ---
    std::string bgHex = node.attribute("background").as_string();
    if (!bgHex.empty()) {
        ctrl->setBackgroundColor(HexToColor(bgHex));
    }

    std::string textColHex = node.attribute("textColor").as_string();
    if (!textColHex.empty()) {
        ctrl->setTextColor(HexToColor(textColHex));
    }

    // --- Font (existent deja) ---
    std::string fontRef = node.attribute("font").as_string();
    if (!fontRef.empty() && m_xmlFontMap.count(fontRef)) {
        auto& fk = m_xmlFontMap[fontRef];
        ctrl->setFont(fk.faceName, fk.height, fk.weight, fk.italic, fk.underline);
    }

    // --- Text Alignment (NOU) ---
    std::string alignStr = node.attribute("textAlign").as_string();
    if (!alignStr.empty()) {
        ctrl->setTextAlign(StringToAlign(alignStr));
    }

    // --- Anchor ---
    std::string anchorStr = node.attribute("anchor").as_string();
    if (!anchorStr.empty()) {
        ctrl->setAnchor(StringToAnchor(anchorStr));
    }
    else {
        // Opțional: setăm un default dacă lipsește în XML
        ctrl->setAnchor(Anchor::LEFT | Anchor::TOP);
    }

    // --- Atribute specifice pentru Layout-uri ---
    // Citim si setam modurile de scalare daca exista
    /*
    std::string wMode = node.attribute("widthMode").as_string(); // ex: "FILL" sau "FIXED"
    if (wMode == "FILL") ctrl->setWidthMode(SizeMode::FILL);
    if (wMode == "FIXED") ctrl->setWidthMode(SizeMode::FIXED);
    if (wMode == "AUTO") ctrl->setWidthMode(SizeMode::AUTO);
    if (wMode == "PERCENT") ctrl->setWidthMode(SizeMode::PERCENT);
    //else ctrl->setWidthMode(SizeMode::FIXED);

    std::string hMode = node.attribute("heightMode").as_string();
    if (hMode == "FILL") ctrl->setHeightMode(SizeMode::FILL);
    if (hMode == "FIXED") ctrl->setHeightMode(SizeMode::FIXED);
    if (hMode == "AUTO") ctrl->setHeightMode(SizeMode::AUTO);
    if (hMode == "PERCENT") ctrl->setHeightMode(SizeMode::PERCENT);
    //else ctrl->setHeightMode(SizeMode::FIXED);
    */

    // --- Atribute specifice pentru Layout-uri ---
    std::string wMode = node.attribute("widthMode").as_string();
    if (wMode == "FILL")         ctrl->setWidthMode(SizeMode::FILL);
    else if (wMode == "FIXED")    ctrl->setWidthMode(SizeMode::FIXED);
    else if (wMode == "AUTO")     ctrl->setWidthMode(SizeMode::AUTO);
    else if (wMode == "PERCENT")  ctrl->setWidthMode(SizeMode::PERCENT); // <--- REPARAT
    else                          ctrl->setWidthMode(SizeMode::FIXED);   // Default de siguranță

    std::string hMode = node.attribute("heightMode").as_string();
    if (hMode == "FILL")         ctrl->setHeightMode(SizeMode::FILL);
    else if (hMode == "FIXED")    ctrl->setHeightMode(SizeMode::FIXED);
    else if (hMode == "AUTO")     ctrl->setHeightMode(SizeMode::AUTO);
    else if (hMode == "PERCENT")  ctrl->setHeightMode(SizeMode::PERCENT); // <--- REPARAT
    else                          ctrl->setHeightMode(SizeMode::FIXED);   // Default de siguranță
    // Grid positions
    ctrl->setGridPosition(
        node.attribute("gridRow").as_int(0),
        node.attribute("gridColumn").as_int(0)
    );

    int minW = node.attribute("minWidth").as_int(0);
    int minH = node.attribute("minHeight").as_int(0);
    if (minW > 0 || minH > 0) {
        ctrl->setMinSize(minW, minH);
    }

    int maxW = node.attribute("maxWidth").as_int(32767);
    int maxH = node.attribute("maxHeight").as_int(32767);
    if (maxW < 32767 || maxH < 32767) {
        ctrl->setMaxSize(maxW, maxH);
    }

    // Citim atributul dbColumn din XML și îl pasăm membrului m_dbColumn din vControl
    std::string dbCol = node.attribute("dbColumn").as_string();
    if (!dbCol.empty()) {
        // Conversia string -> wstring se face prin str_to_wstr (pe care am văzut că o folosești deja)
        ctrl->setAttribute(L"dbColumn",str_to_wstr(dbCol));

        //LOG_DEBUG(L"[XML] Mapare DB: Control '" + str_to_wstr(ctrl->getId()) +
          //  L"' -> Coloana '" + ctrl->getAttribute(L"dbColumn") + L"'");
    }
   
}


void vXmlDialog::applySpecificAttribute(vControl* ctrl, const std::string& name, const std::string& value) {
    
    if (name == "readOnly") {
        vEdit* edit = dynamic_cast<vEdit*>(ctrl);
        if (edit) {
            bool isRO = (value == "true" || value == "1");
            edit->setReadOnly(isRO);
            //LOG_DEBUG(L"[XML] Setat ReadOnly=" + std::to_wstring(isRO) + L" pentru " + str_to_wstr(ctrl->getId()));
        }
        vDbGridPicker* gridPiker = dynamic_cast<vDbGridPicker*>(ctrl);
        if (gridPiker) {
            bool isRO = (value == "true" || value == "1");
            gridPiker->setReadOnly(isRO);
            //LOG_DEBUG(L"[XML] Setat ReadOnly=" + std::to_wstring(isRO) + L" pentru " + str_to_wstr(ctrl->getId()));
        }
    }
    else if(name == "onlyDigits"){
        vEdit* edit = dynamic_cast<vEdit*>(ctrl);
        if (edit) {
            bool isRO = (value == "true" || value == "1");
            edit->setOnlyDigits(isRO);
            //LOG_DEBUG(L"[XML] Setat ReadOnly=" + std::to_wstring(isRO) + L" pentru " + str_to_wstr(ctrl->getId()));
        }
    }
    else if (name == "dateFormat") {
        vDatePicker* dp = dynamic_cast<vDatePicker*>(ctrl);
        if (dp) {
            // Trebuie să adaugi o metodă setFormat(wstring) în vDatePicker
            dp->setFormat(str_to_wstr(value));
        }
    }
    else if (name == "title") {
        vPanelGroup* vpg = dynamic_cast<vPanelGroup*>(ctrl);
        if (vpg) {
            //LOG_DEBUG(L"Titlu setat: " + str_to_wstr(value));
            vpg->setTitle(str_to_wstr(value));
        }
        else {
            LOG_ERROR(L"Cast esuat! Controlul nu este vPanelGroup.");
        }
    }
    else if (name == "borderColor") {
        //LOG_DEBUG(L"Încercare setare borderColor pentru " + str_to_wstr(ctrl->getId()));
        vPanelGroup* vpg = dynamic_cast<vPanelGroup*>(ctrl);
        if (vpg) {
            //LOG_DEBUG(L"Cast reusit! Culoare: " + str_to_wstr(value));
            COLORREF clHex = HexToColor(value); // Lipsea punctul și virgula aici
            vpg->setBorderColor(clHex);
        }
        else {
            LOG_ERROR(L"Cast esuat! Controlul nu este vPanelGroup.");
        }
    }else if (name == "targetQuery") {
        vDbGridPicker* gridPick = dynamic_cast<vDbGridPicker*>(ctrl);
        if (gridPick) {
            gridPick->setTargetQuery(str_to_wstr(value));
        }
        vDbComboBox* dbCombo = dynamic_cast<vDbComboBox*>(ctrl);
        if (dbCombo) {
            dbCombo->setTargetQuery(str_to_wstr(value));
        }
    }
    else if (name == "returnColumn") {
        vDbGridPicker* gridPick = dynamic_cast<vDbGridPicker*>(ctrl);
        if (gridPick) {
            gridPick->setReturnColumn(str_to_wstr(value));
        }
    }
    else if (name == "returnIdColumn") {
        vDbGridPicker* gridPick = dynamic_cast<vDbGridPicker*>(ctrl);
        if (gridPick) {
            gridPick->setReturnIdColumn(str_to_wstr(value));
        }
    }
   
    else if (name == "editType") {
        vEdit* edit = dynamic_cast<vEdit*>(ctrl);
        if (edit) {
            if (to_upper(value) == "SINGLE_LINE") edit->setEditType(EditType::SINGLE_LINE);
            if (to_upper(value) == "MULTI_LINE") edit->setEditType(EditType::MULTI_LINE);
            if (to_upper(value) == "CONSOLE_LINE") edit->setEditType(EditType::CONSOLE_LINE);
            if (to_upper(value) == "PASSWORD") edit->setEditType(EditType::PASSWORD);
        }
    }
    
}


bool vXmlDialog::isFormValid() {
    bool allOk = true;

    for (auto& pair : m_children) {
        // Apelăm versiunea recursivă
        if (!pair.second->validateRecursive()) {
            allOk = false;
        }
    }

    return allOk;
}

#include "PdfConverter.hpp"

void PdfConverter::applyCssToStyle(const XhtmlElement& element, Style& target_style) {

    // 1. Obținerea proprietăților CSS (rezolvarea cascadei)
    // Presupunem că m_xhtml.getStylesForElement rezolvă CSS Cascade și include stilurile inline.
    std::map<std::wstring, std::wstring> properties = m_xhtml.getStylesForElement(element);

    // Determină font-size-ul părintelui (similar cu logica din applyCssToContext).
    // Folosim font-size-ul structurii de stil pe care o modificăm, ca bază.
    double parent_font_size = target_style.fontSize; // Folosim target_style ca bază

    // Referință la dimensiunea fontului curent (se va actualiza la Pasul 1)
    double& current_font_size = target_style.fontSize;

    // --- Lambda utilitar pentru conversia lungimilor (Box Model, Width, etc.) ---
    // Aceasta folosește funcția cu 2 argumente și font-size-ul CURENT (al copilului, adică target_style.fontSize)
    // IMPORTANT: Această lambda trebuie definită aici pentru a folosi current_font_size (referința la target_style.fontSize)
    auto GET_LENGTH_PT = [&](const std::wstring& value) -> double {
        return ConvertUtils::convertCssLengthToPt(value, current_font_size);
    };

    if (properties.count(L"rule-name")) {
        target_style.ruleName = properties.at(L"rule-name");
    }

    // ==========================================================
    // PASUL 1: FONTURI ȘI TEXT (Prioritate: font-size)
    // ==========================================================

    // A. font-size (PRIORITAR)
    if (properties.count(L"font-size")) {
        // Folosește funcția specializată care se bazează pe font-size-ul PĂRINTELUI.
        current_font_size = ConvertUtils::convertCssFontLengthToPt(
            properties.at(L"font-size"),
            parent_font_size // Folosim font-size-ul vechi al target_style ca părinte
        );
    }

    // B. Alte proprietăți de Font și Text
    // ... (Logica de copiere a proprietăților font/text din funcția originală, dar pe target_style) ...
    if (properties.count(L"font-family")) {
        target_style.fontFamily = properties.at(L"font-family");
    }
    // ... (continuă cu font-weight, font-style, line-height, etc. pe target_style) ...
    if (properties.count(L"font-weight")) {
        target_style.fontWeight = properties.at(L"font-weight");
    }
    if (properties.count(L"font-style")) {
        target_style.fontStyle = properties.at(L"font-style");
    }
    if (properties.count(L"line-height")) {
        double val_pt = ConvertUtils::convertCssFontLengthToPt(properties.at(L"line-height"), current_font_size);
        if (current_font_size > 0.0) {
            target_style.lineHeight = val_pt / current_font_size;
        }
        if (target_style.lineHeight < 0.1) target_style.lineHeight = 1.2;
    }
    if (properties.count(L"letter-spacing")) {
        target_style.letterSpacing = GET_LENGTH_PT(properties.at(L"letter-spacing"));
    }
    if (properties.count(L"word-spacing")) {
        target_style.wordSpacing = GET_LENGTH_PT(properties.at(L"word-spacing"));
    }
    if (properties.count(L"text-decoration")) {
        target_style.textDecoration = properties.at(L"text-decoration");
    }
    if (properties.count(L"text-align")) {
        target_style.textAlign = properties.at(L"text-align");
    }
    if (properties.count(L"vertical-align")) {
        target_style.verticalAlign = properties.at(L"vertical-align");
    }


    // ==========================================================
    // PASUL 2: CULORI ȘI FUNDAL
    // ==========================================================

    if (properties.count(L"color")) {
        target_style.textColor = ConvertUtils::parseCssColorToRgb(properties.at(L"color"));
    }
    if (properties.count(L"background-color")) {
        target_style.backgroundColor = ConvertUtils::parseCssColorToRgb(properties.at(L"background-color"));
    }


    // ==========================================================
    // PASUL 3: BOX MODEL (Margin, Padding, Border)
    // ==========================================================

    // NOTĂ: Proprietățile Box Model ar trebui aplicate, mai ales pentru citirea Padding/Border
    // necesare pentru a calcula Lățimea/Înălțimea de conținut a celulei.

    // --- Shorhands ---
    if (properties.count(L"margin")) {
        ConvertUtils::parseBoxShorthand(properties.at(L"margin"), current_font_size,
            target_style.boxModel.marginTop, target_style.boxModel.marginRight,
            target_style.boxModel.marginBottom, target_style.boxModel.marginLeft);
    }
    // ... (continuați cu toate celelalte proprietăți Box Model pe target_style) ...
    if (properties.count(L"padding")) {
        ConvertUtils::parseBoxShorthand(properties.at(L"padding"), current_font_size,
            target_style.boxModel.paddingTop, target_style.boxModel.paddingRight,
            target_style.boxModel.paddingBottom, target_style.boxModel.paddingLeft);
    }

    if (properties.count(L"border")) {
        std::wstring temp_style;
        double temp_width;
        ColorRgb temp_color;

        ConvertUtils::parseBorderShorthand(properties.at(L"border"),
            temp_style, temp_width, temp_color, current_font_size);

        // Aplicăm pe toate laturile
        target_style.boxModel.borderTopWidth = target_style.boxModel.borderRightWidth =
            target_style.boxModel.borderBottomWidth = target_style.boxModel.borderLeftWidth = temp_width;

        target_style.borderStyle = temp_style;
        target_style.borderColor = temp_color;
    }

    // --- Proprietăți individuale (Margin/Padding/Border Width) ---
    // (Este crucial să le includeți pe toate, dar iată un exemplu pentru width și height, care sunt vitale pentru tabele)

    // Margin (T, R, B, L)
    if (properties.count(L"margin-top")) { target_style.boxModel.marginTop = GET_LENGTH_PT(properties.at(L"margin-top")); }
    // ...

    // Padding (T, R, B, L)
    if (properties.count(L"padding-top")) { target_style.boxModel.paddingTop = GET_LENGTH_PT(properties.at(L"padding-top")); }
    if (properties.count(L"padding-right")) { target_style.boxModel.paddingRight = GET_LENGTH_PT(properties.at(L"padding-right")); }
    if (properties.count(L"padding-bottom")) { target_style.boxModel.paddingBottom = GET_LENGTH_PT(properties.at(L"padding-bottom")); }
    if (properties.count(L"padding-left")) { target_style.boxModel.paddingLeft = GET_LENGTH_PT(properties.at(L"padding-left")); }

    // Border Width (T, R, B, L)
    if (properties.count(L"border-top-width")) { target_style.boxModel.borderTopWidth = GET_LENGTH_PT(properties.at(L"border-top-width")); }
    if (properties.count(L"border-right-width")) { target_style.boxModel.borderRightWidth = GET_LENGTH_PT(properties.at(L"border-right-width")); }
    if (properties.count(L"border-bottom-width")) { target_style.boxModel.borderBottomWidth = GET_LENGTH_PT(properties.at(L"border-bottom-width")); }
    if (properties.count(L"border-left-width")) { target_style.boxModel.borderLeftWidth = GET_LENGTH_PT(properties.at(L"border-left-width")); }

    // Border Style/Color
    if (properties.count(L"border-style")) { target_style.borderStyle = properties.at(L"border-style"); }
    if (properties.count(L"border-color")) { target_style.borderColor = ConvertUtils::parseCssColorToRgb(properties.at(L"border-color")); }


    // ==========================================================
    // PASUL 4: LAYOUT ȘI POZIȚIONARE
    // ==========================================================

    // Dimensiuni (CRUCIAL pentru <tr> și <td>)
    if (properties.count(L"width")) {
        target_style.width = GET_LENGTH_PT(properties.at(L"width"));
    }
    if (properties.count(L"height")) {
        target_style.height = GET_LENGTH_PT(properties.at(L"height"));
    }

    // Alte proprietăți de layout
    if (properties.count(L"display")) {
        target_style.display = properties.at(L"display");
    }
    // ... (restul proprietăților de layout, dacă sunt necesare) ...
}


void PdfConverter::applyCssToContext(const XhtmlElement& element) {

    // 1. Obținerea proprietăților CSS (rezolvarea cascadei)
    // Presupunem că m_xhtml.getStylesForElement rezolvă CSS Cascade și include stilurile inline.
    std::map<std::wstring, std::wstring> properties = m_xhtml.getStylesForElement(element);

    // Determină font-size-ul părintelui (necesar pentru conversia 'em' la font-size).
    // Dacă stiva este goală (elementul rădăcină), luăm valoarea curentă moștenită.
    double parent_font_size = m_contextStack.empty()
        ? m_context.style.fontSize
        : m_contextStack.top().style.fontSize;

    // Referință la dimensiunea fontului curent (se va actualiza la Pasul 1)
    double& current_font_size = m_context.style.fontSize;

    // --- Lambda utilitar pentru conversia lungimilor (Box Model, Width, etc.) ---
    // Aceasta folosește funcția cu 2 argumente și font-size-ul CURENT (al copilului)
    auto GET_LENGTH_PT = [&](const std::wstring& value) -> double {
        return ConvertUtils::convertCssLengthToPt(value, current_font_size);
    };


    if (properties.count(L"rule-name")) {
        m_context.style.ruleName = properties.at(L"rule-name");
    }


    // ==========================================================
    // PASUL 1: FONTURI ȘI TEXT (Prioritate: font-size)
    // ==========================================================




    // A. font-size (PRIORITAR)
    if (properties.count(L"font-size")) {
        // Folosește funcția specializată care se bazează pe font-size-ul PĂRINTELUI.
        current_font_size = ConvertUtils::convertCssFontLengthToPt(
            properties.at(L"font-size"),
            parent_font_size
        );
    }

    // B. Alte proprietăți de Font și Text
    if (properties.count(L"font-family")) {
        m_context.style.fontFamily = properties.at(L"font-family");
    }
    if (properties.count(L"font-weight")) {
        m_context.style.fontWeight = properties.at(L"font-weight");
    }
    if (properties.count(L"font-style")) {
        m_context.style.fontStyle = properties.at(L"font-style");
    }
    if (properties.count(L"line-height")) {
        // Line-height este stocat ca FACTOR (ex: 1.2), de aceea împărțim rezultatul la font-size.
        double val_pt = ConvertUtils::convertCssFontLengthToPt(properties.at(L"line-height"), current_font_size);
        if (current_font_size > 0.0) {
            m_context.style.lineHeight = val_pt / current_font_size;
        }
        if (m_context.style.lineHeight < 0.1) m_context.style.lineHeight = 1.2;
    }
    if (properties.count(L"letter-spacing")) {
        m_context.style.letterSpacing = GET_LENGTH_PT(properties.at(L"letter-spacing"));
    }
    if (properties.count(L"word-spacing")) {
        m_context.style.wordSpacing = GET_LENGTH_PT(properties.at(L"word-spacing"));
    }
    if (properties.count(L"text-decoration")) {
        m_context.style.textDecoration = properties.at(L"text-decoration");
    }
    if (properties.count(L"text-align")) {
        m_context.style.textAlign = properties.at(L"text-align");
    }


    // ==========================================================
    // PASUL 2: CULORI ȘI FUNDAL
    // ==========================================================

    if (properties.count(L"color")) {
        m_context.style.textColor = ConvertUtils::parseCssColorToRgb(properties.at(L"color"));
    }
    if (properties.count(L"background-color")) {
        m_context.style.backgroundColor = ConvertUtils::parseCssColorToRgb(properties.at(L"background-color"));
    }


    // ==========================================================
    // PASUL 3: BOX MODEL (Margin, Padding, Border)
    // ==========================================================

    // --- Shorhands (Utilizează current_font_size) ---

    // 1. Margin Shorthand
    if (properties.count(L"margin")) {
        ConvertUtils::parseBoxShorthand(properties.at(L"margin"), current_font_size,
            m_context.style.boxModel.marginTop, m_context.style.boxModel.marginRight,
            m_context.style.boxModel.marginBottom, m_context.style.boxModel.marginLeft);
    }

    // 2. Padding Shorthand
    if (properties.count(L"padding")) {
        ConvertUtils::parseBoxShorthand(properties.at(L"padding"), current_font_size,
            m_context.style.boxModel.paddingTop, m_context.style.boxModel.paddingRight,
            m_context.style.boxModel.paddingBottom, m_context.style.boxModel.paddingLeft);
    }

    // 3. Border Shorthand
    if (properties.count(L"border")) {
        std::wstring temp_style;
        double temp_width;
        ColorRgb temp_color;

        ConvertUtils::parseBorderShorthand(properties.at(L"border"),
            temp_style, temp_width, temp_color, current_font_size);

        // Aplicăm pe toate laturile
        m_context.style.boxModel.borderTopWidth = m_context.style.boxModel.borderRightWidth =
            m_context.style.boxModel.borderBottomWidth = m_context.style.boxModel.borderLeftWidth = temp_width;

        m_context.style.borderStyle = temp_style;
        m_context.style.borderColor = temp_color;
    }


    // --- Individual Properties (Suprascriu shorthands, dacă există) ---

    // Margin
    if (properties.count(L"margin-top")) {
        m_context.style.boxModel.marginTop = GET_LENGTH_PT(properties.at(L"margin-top"));
    }
    if (properties.count(L"margin-right")) {
        m_context.style.boxModel.marginRight = GET_LENGTH_PT(properties.at(L"margin-right"));
    }
    if (properties.count(L"margin-bottom")) {
        m_context.style.boxModel.marginBottom = GET_LENGTH_PT(properties.at(L"margin-bottom"));
    }
    if (properties.count(L"margin-left")) {
        m_context.style.boxModel.marginLeft = GET_LENGTH_PT(properties.at(L"margin-left"));
    }

    // Padding
    if (properties.count(L"padding-top")) {
        m_context.style.boxModel.paddingTop = GET_LENGTH_PT(properties.at(L"padding-top"));
    }
    if (properties.count(L"padding-right")) {
        m_context.style.boxModel.paddingRight = GET_LENGTH_PT(properties.at(L"padding-right"));
    }
    if (properties.count(L"padding-bottom")) {
        m_context.style.boxModel.paddingBottom = GET_LENGTH_PT(properties.at(L"padding-bottom"));
    }
    if (properties.count(L"padding-left")) {
        m_context.style.boxModel.paddingLeft = GET_LENGTH_PT(properties.at(L"padding-left"));
    }

    // Border Width
    if (properties.count(L"border-top-width")) {
        m_context.style.boxModel.borderTopWidth = GET_LENGTH_PT(properties.at(L"border-top-width"));
    }
    if (properties.count(L"border-right-width")) {
        m_context.style.boxModel.borderRightWidth = GET_LENGTH_PT(properties.at(L"border-right-width"));
    }
    if (properties.count(L"border-bottom-width")) {
        m_context.style.boxModel.borderBottomWidth = GET_LENGTH_PT(properties.at(L"border-bottom-width"));
    }
    if (properties.count(L"border-left-width")) {
        m_context.style.boxModel.borderLeftWidth = GET_LENGTH_PT(properties.at(L"border-left-width"));
    }

    // Border Style/Color
    if (properties.count(L"border-style")) {
        m_context.style.borderStyle = properties.at(L"border-style");
    }
    if (properties.count(L"border-color")) {
        m_context.style.borderColor = ConvertUtils::parseCssColorToRgb(properties.at(L"border-color"));
    }


    // ==========================================================
    // PASUL 4: LAYOUT ȘI POZIȚIONARE
    // ==========================================================

    // Dimensiuni
    if (properties.count(L"width")) {
        m_context.style.width = GET_LENGTH_PT(properties.at(L"width"));
    }
    if (properties.count(L"height")) {
        m_context.style.height = GET_LENGTH_PT(properties.at(L"height"));
    }
    if (properties.count(L"max-width")) {
        m_context.style.maxWidth = GET_LENGTH_PT(properties.at(L"max-width"));
    }
    if (properties.count(L"min-height")) {
        m_context.style.minHeight = GET_LENGTH_PT(properties.at(L"min-height"));
    }

    // Flow & Poziționare
    if (properties.count(L"display")) {
        m_context.style.display = properties.at(L"display");
    }
    else { // setez valoarea default
        const std::wstring& tagName = element.getTagName();
        auto it = HtmlDefaultDisplay.find(tagName);

        if (it != HtmlDefaultDisplay.end()) {
            // S-a găsit valoarea implicită pentru tag
            m_context.style.display = it->second;
            LOG_DEBUG(L"[DEFAULT] Display setat default: " + m_context.style.display);
        }
        else {
            // C. Valoare de rezervă (Fallback)
            // Pentru tag-uri necunoscute sau text-nodes. Text nodes ar trebui să fie 'inline' implicit.
            m_context.style.display = (element.getTagName() == L"#text") ? L"inline" : L"block";
            LOG_WARNING(L"[DEFAULT] Display necunoscut pentru <" + tagName + L">. Se folosește fallback: " + m_context.style.display);
        }
    }

    if (properties.count(L"position")) {
        m_context.style.position = properties.at(L"position");
    }
    if (properties.count(L"top")) {
        m_context.style.top = GET_LENGTH_PT(properties.at(L"top"));
    }
    if (properties.count(L"left")) {
        m_context.style.left = GET_LENGTH_PT(properties.at(L"left"));
    }
    if (properties.count(L"right")) {
        m_context.style.right = GET_LENGTH_PT(properties.at(L"right"));
    }
    if (properties.count(L"bottom")) {
        m_context.style.bottom = GET_LENGTH_PT(properties.at(L"bottom"));
    }
    if (properties.count(L"vertical-align")) {
        m_context.style.verticalAlign = properties.at(L"vertical-align");
    }





}
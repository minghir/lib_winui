#include "PdfConverter.hpp" // Aici trebuie sa incluzi si Xhtml.hpp, PdfWriterWrapper.hpp, RenderingContext.hpp, Page.hpp
#include "ConvertUtils.hpp"
#include "../ui/ConsoleManager.hpp"
#include "ContextGuard.hpp"

#include <iomanip>


PdfConverter::PdfConverter(const Xhtml& xhtml)
// 1. Inițializarea membrilor (Lista de inițializare)
// m_xhtml și m_pdfWriter sunt inițializate.
// m_currentPage folosește implicit Page(L"A4", Portrait, 72.0, 1)
    : m_xhtml(xhtml),
    m_currentPage(L"A4", PageOrientation::Portrait, 72.0, 1),

    // 2. Inițializarea Contextului
    // M_context primește adresa lui m_currentPage
    m_context(&m_currentPage, &this->m_xhtml.getRoot())
{
    // 3. Setări Finale (Opțional)
    // Dacă ai reguli CSS @page în Xhtml, le aplici aici.
    // Acesta este un pas important, deoarece @page setează marginile
    // care sunt esențiale pentru cursor_y inițial.

    const auto& page_rules = m_xhtml.getStyles().getRule(L"@page");
    if (!page_rules.empty()) {
        m_currentPage.applyCssPageRule(page_rules);
        // După aplicarea CSS-ului, trebuie să reinițializăm contextul
        // pentru a prelua noile margini și dimensiuni.
        m_context = RenderingContext(&m_currentPage, &this->m_xhtml.getRoot());
    }

    //printRenderingContext();
    //LOG_DEBUG(L"m_context.cursor_y" + to_wstring(m_context.cursor_y) + L" m_currentPage.getMarginTop()" + to_wstring(m_currentPage.getMarginTop()));
    // M_pdfWriter va fi inițializat în metoda convert() când se deschide fișierul.

    // Verificare rapidă: m_context ar trebui să fie gata de start
    // Aici m_context.cursor_y ar trebui să fie egal cu m_currentPage.getMarginTop()
}

// PdfConverter.cpp

bool PdfConverter::convert(const std::wstring& outputFilePath) {

    // 1. Initializare Context și Aplicare Stiluri @page
    while (!m_contextStack.empty()) { m_contextStack.pop(); }

    // Creează un context gol, care va fi reinițializat mai jos
    // Aici nu avem încă referința la m_currentPage, dar e necesară pentru Page::applyCssPageRule
    Page initialPage; // O variabilă locală Page temporară, doar pentru a prelua stilurile
    std::map<std::wstring, std::wstring> pageProperties = m_xhtml.getStyles().getRule(L"@page");
    initialPage.applyCssPageRule(pageProperties);

    // ⭐ MUTARE ȘI CORECȚIE: Aliniere la m_pdfWriter.initialize(path, width, height)
    // Acum avem dimensiunile corecte din obiectul 'initialPage' configurat.
    if (!m_pdfWriter.initialize(outputFilePath, initialPage.getWidth(), initialPage.getHeight())) {
        // Logica de eroare
        return false;
    }

    // Asignăm Page-ul local la membrul clasei m_currentPage (pe care îl vom folosi în restul procesului)
    m_currentPage = initialPage;
    m_currentPage.print();
    // 2. Inițializare Context Global (RenderingContext)
    // Ne bazăm pe constructorul tău: RenderingContext(const Page* page)
    m_context = RenderingContext(&m_currentPage, &this->m_xhtml.getRoot());
    m_context.print(L"INCEPUT");
    // Initializarea cursorilor (setată deja de constructorul RenderingContext)
    // m_context.cursor_y = m_currentPage.getHeight() - m_currentPage.getMarginTop();
    // m_context.cursor_x = m_currentPage.getMarginLeft();

    // 3. Adăugarea Primei Pagini în Document
    // ⭐ CORECȚIE: Folosim m_pdfWriter.beginPage (așa cum am convenit)

   // m_pdfWriter.beginPage(m_currentPage);

    // 4. Lansarea Procesării Recursive
    XhtmlElement root = m_xhtml.getRoot();
    
    m_context.cursor_y = m_currentPage.getHeight() - m_currentPage.getMarginTop();
    LOG_DEBUG(L"AM SETAT cursor_y:" + to_wstring<double>(m_currentPage.getHeight()) + L"-" + to_wstring<double>(m_currentPage.getMarginTop())+L"=" + to_wstring<double>(m_context.cursor_y));
    processNodeRecursive(root);

    finalizeAndPaint();
    //paintPage(m_currentPage);
    // 5. Finalizare Document
    

    return true;
}




void PdfConverter::processTextContent(const std::wstring& textContent) {

    // 1. Pregătirea datelor și a contextului
    std::vector<std::wstring> tokens = ConvertUtils::tokenizeText(textContent);
    const Style& currentStyle = m_context.style;

    double flow_total_width = m_context.metrics.content_width;
    double flow_end_x = m_context.metrics.x_content_start + flow_total_width;

    LOG_DEBUG(L"[TEXT PROCESS] Incepe procesare text: '" + textContent.substr(0, std::min<int>((int)textContent.length(), 30)) + L"'");

    // 1.1. Tratarea spațiilor leading/trailing generate de elementul block/inline-block
    if (m_context.current_xhtml_element->getAttribute(L"__xhtml_leading_space") == L"true") {
        tokens.insert(tokens.begin(), L" ");
    }
    if (m_context.current_xhtml_element->getAttribute(L"__xhtml_trailing_space") == L"true") {
        tokens.push_back(L" ");
    }

    // 1.2. Pre-procesarea &nbsp; (Substituirea cu spațiu normal L" ")
    // Această buclă folosește 'auto&' pentru a modifica elementele din vector.
   
    // --- 2. Iterarea și Plasarea Token-urilor (Buclă cu re-încercare) ---

    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& token = tokens[i];

        // Calculează lățimea rămasă (poate fi actualizată după un wrap/page break)
        double remainingWidth = flow_end_x - m_context.cursor_x;

        LOG_DEBUG(L"[TOKEN] Analiza: '" + token + L"'");

        // 2.1. Logica Spațiilor (Colapsare și Ignorare)
        if (token == L" ") {
            // Ignorare la început de linie
            if (m_lineBuffer.empty()) {
                LOG_DEBUG(L"[SKIP] Spațiu ignorat (început de linie).");
                continue; // Treci la următorul token
            }

            // Colapsare spații consecutive
            if (!m_lineBuffer.empty() && m_lineBuffer.back().text == L" ") {
                LOG_DEBUG(L"[SKIP] Spațiu ignorat (colapsare spații).");
                continue; // Treci la următorul token
            }
        }

        // Măsurarea Lățimii Token-ului
        double tokenWidth = m_pdfWriter.measureTextWidth(token, currentStyle);
        LOG_DEBUG(L"[MEASURE] Lățime măsurată: " + std::to_wstring(tokenWidth));

        // --- 3. LOGICA WORD-WRAPPING ---

        // Dacă token-ul curent nu încape ȘI NU este un spațiu (spațiul normal nu poate forța wrap, ci doar completează linia)
        if (tokenWidth > remainingWidth && token != L" ") {
            LOG_ERROR(L"[WRAP DEBUG] Wrap necesar! Token: '" + token +
                L"', Width: " + std::to_wstring(tokenWidth) +
                L", Remaining: " + std::to_wstring(remainingWidth) +
                L", Context_y: " + to_wstring(m_context.cursor_y) +
                L", Content_height: " + to_wstring(m_context.metrics.content_height) +
                L", Buffer empty: " + (m_lineBuffer.empty() ? L"TRUE" : L"FALSE"));
            // Se face wrap doar dacă linia are deja conținut
            if (!m_lineBuffer.empty()) {
                LOG_DEBUG(L"[WRAP] Cuvântul nu încape. Se trece la linia nouă.");

                // Finalizează linia curentă și treci la următoarea
                newLineAndCheckPageBreak();

                // Scădem i pentru a RE-PROCESA token-ul curent la următoarea iterație (pe noua linie)
                --i;
                continue; // Sarim la următoarea iterație
            }
            // Cazul Else: Cuvântul este mai mare decât întreaga lățime a liniei. Îl lăsăm să depășească.
        }

        // --- 4. RANDAREA ȘI AVANSAREA CURSORULUI ---

        // Pregătește Token-ul pentru buffer
        LineToken currentToken;
        currentToken.text = token;
        currentToken.width = tokenWidth;
        currentToken.style = m_context.style;

        // Adăugăm Token-ul în Buffer
        m_lineBuffer.push_back(currentToken);
        LOG_DEBUG(L"[RENDER_LINE_BUFFER] Se adaugă '" + token + L"' la buffer.");

        // Avansarea Cursorului
        m_context.cursor_x += tokenWidth;

        LOG_DEBUG(L"[ADVANCE] X nou: " + std::to_wstring(m_context.cursor_x));
    }
}

void PdfConverter::processLineBreak() {

    double line_height_pt = m_context.style.lineHeight * m_context.style.fontSize;

    // 1. Verificare Paginare (Se face inainte de a avansa cursorul)
    // Presupunând că 'm_currentPage.getBottomMarginY()' este înălțimea Y absolută (ex: 100)
    if (m_context.cursor_y - line_height_pt < m_currentPage.getMarginBottom()) {


        // Logica de Paginare:
        //m_pdfWriter.startPage();
        //m_pdfWriter.beginPage(m_currentPage);
        //newPage();
        checkPageBreak(m_context.cursor_y - line_height_pt);

        // Resetează cursorul Y la Top Margin, apoi avansează pentru noua linie
        m_context.cursor_y = m_currentPage.getHeight() - m_currentPage.getMarginTop();
    }

    newLineAndCheckPageBreak();
    // 2. Mută cursorul Y (SCAZĂ valoarea pentru a coborî pe pagină)
    m_context.cursor_y -= line_height_pt;

    // 3. Resetează cursorul X la marginea de start a flow-ului
    m_context.cursor_x = m_context.metrics.x_content_start;

    // 4. Resetează flag-urile
    //m_context.current_line_has_content = false;
    //m_context.last_item_was_space = false;
}


void PdfConverter::newLineAndCheckPageBreak() {

    // 1. VERIFICARE STARE
    // Dacă nu avem conținut pe linia curentă, nu facem nimic, dar resetăm cursorul X.
    if (m_lineBuffer.empty()) { // <-- MODIFICAT AICI
        LOG_DEBUG(L"[NEW LINE] Apel ignorat: Linia curentă este goală.");
        // Resetează cursorul X la punctul de start al conținutului box-ului curent.
        m_context.cursor_x = m_context.metrics.x_content_start;
        // Asigură-te că și flag-ul este resetat (deși nu ar trebui să fie necesar dacă e gol)
        //m_context.current_line_has_content = false;
        return;
    }
    /*
    if (!m_context.current_line_has_content) {
        LOG_DEBUG(L"[NEW LINE] Apel ignorat: Linia curentă este goală.");
        // Resetează cursorul X la punctul de start al conținutului box-ului curent.
        m_context.cursor_x = m_context.metrics.x_content_start;
        return;
    }
    */
    //LOG_DEBUG(L"--- [NEW LINE] Finalizare linie și avansare Y. ---");

    // 2. CALCULE ALINIERE (X Offset)

    // Lățimea totală a textului randat pe linia curentă.
    // Aceasta este distanța dintre poziția finală a cursorului și poziția de start a liniei.
    //double line_content_width = m_context.cursor_x - m_context.current_line_start_x;

    double line_content_width = 0.0;
    for (const auto& token : m_lineBuffer) {
        line_content_width += token.width;
    }

    //double line_content_width = m_context.cursor_x - m_context.current_line_start_x;

    // Lățimea disponibilă maximă (este lățimea conținutului box-ului curent).
    // Folosim m_context.metrics.content_width direct.
    double flow_width = m_context.metrics.content_width;

    // Spațiul rămas pe care trebuie să-l umplem.
    double available_space = flow_width - line_content_width;
    double offset_x = 0.0;

    // Calculul offset-ului X pentru a centra/aliniere la dreapta.
    const std::wstring& align = m_context.style.textAlign;

    if (align == L"center") {
        offset_x = available_space / 2.0;
        LOG_DEBUG(L"[ALIGN] Centrare. Offset X: " + std::to_wstring(offset_x));
    }
    else if (align == L"right") {
        offset_x = available_space;
        LOG_DEBUG(L"[ALIGN] Dreapta. Offset X: " + std::to_wstring(offset_x));
    }
    // NOTĂ: Pentru 'left' și 'justify', offset_x rămâne 0.0.

    //double current_token_x = m_context.current_line_start_x + offset_x;
    double current_token_x = m_context.metrics.x_content_start + offset_x;
    //double line_height_pt = calculateLineHeight();
    double line_height_pt = m_context.style.fontSize * m_context.style.lineHeight;

    m_context.cursor_y -= line_height_pt;

    LOG_ERROR(L"NOUL Y:" + to_wstring<double>(m_context.cursor_y));
    // 3. RANDARE LINIE
    // Presupunând că m_pdfWriter.renderLineBuffer ia în considerare m_context.current_line_start_x + offset_x
    // ca poziție de bază.

    for (const auto& token : m_lineBuffer) {
        RenderInstruction instruction;
        instruction.x = current_token_x;
        instruction.y = m_context.cursor_y; // Coordonata Y de jos a bordurii (pentru randarea PDF)
        //instruction.y = m_context.cursor_y + m_context.style.fontSize;

        //instruction.width = m_context.metrics.width;
        instruction.width = token.width; // Lățimea token-ului (cuvânt sau spațiu)
        // Înălțimea este, de obicei, line_height_pt sau înălțimea fontului.
        instruction.height = m_context.style.fontSize * m_context.style.lineHeight;

        instruction.text_content = token.text;
        instruction.z_order = m_context.depth + 1;;
        //instruction.style = m_context.style;
        instruction.style = token.style;
        instruction.renderFunction = L"text";
        m_currentPage.pushBackInstruction(instruction);
        

        current_token_x += token.width;
    }
   

    LOG_DEBUG(L"[RENDER LINE] Simulare randare linie la Y: " + std::to_wstring(m_context.cursor_y) + L" cu Offset X: " + std::to_wstring(offset_x));


    // 4. AVANSARE Y ȘI VERIFICARE PAGINARE

    // Calculează înălțimea liniei pe baza font-size și line-height.
    

    // Scade Y (deoarece Y crește în jos, iar în PDF crește de jos în sus,
    // scăderea mută cursorul mai "jos" pe ecran, adică spre marginea inferioară).
   
    //m_context.cursor_y -= line_height_pt;

    LOG_DEBUG(L"[ADVANCE Y] Y nou: " + std::to_wstring(m_context.cursor_y) + L". Înălțime linie: " + std::to_wstring(line_height_pt));


    // ⭐ VERIFICARE PAGINARE
    // Verifică dacă noul cursor Y este sub limita de jos a paginii.
    
    if (m_context.cursor_y <= (  m_currentPage.getMarginBottom() )) {
        LOG_DEBUG(L"[PAGE BREAK] Limita inferioară atinsă. Se adaugă pagină nouă.");
        //finalizeAndPaint();
        // Aici se apelează m_pdfWriter.startNewPage()
         //m_pdfWriter.beginPage(m_currentPage);
        //newPage();
        //checkPageBreak(m_currentPage.getMarginBottom());
         // Resetează cursorul Y la marginea de sus a noii pagini
         m_context.cursor_y = m_currentPage.getHeight() - m_currentPage.getMarginTop();


        // Dacă ai elemente pe mai multe pagini (ex: un bloc mare), trebuie tratată și
        // logica de întrerupere a blocului curent și reînceperea pe pagina următoare, 
        // dar pentru un flow simplu se resetează doar cursorul Y.
    }
    
    // 5. RESETARE STARE LINIE NOUĂ
    // Resetează X la punctul de start al conținutului Box-ului curent.
    m_lineBuffer.clear();

    m_context.cursor_x = m_context.metrics.x_content_start;
   // m_context.current_line_start_x = m_context.metrics.x_content_start; // Începe linia nouă de la aceeași poziție.
    //m_context.current_line_has_content = false;
    //m_context.last_item_was_space = false;
    // (Ar trebui să se golească și m_lineBuffer aici)

    LOG_DEBUG(L"--- [NEW LINE] Linia noua începe la X: " + std::to_wstring(m_context.cursor_x));
}




void PdfConverter::processTextNode(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processTextNode - START");
    const std::wstring& textContent = element.getTagContent();
    m_context.current_xhtml_element = &element;
    if (!textContent.empty()) {
        // ⭐️ Adaugă textul în buffer-ul liniei, unde se va măsura și se va alinia.
        processTextContent(textContent);
    }
    LOG_INFO(L"PdfConverter::processTextNode - END");
}

void PdfConverter::processBody(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processBody - START");
    processGenericBlock(element); // Singurul apel rămas
    LOG_INFO(L"PdfConverter::processBody - END");
}

void PdfConverter::processHtml(const XhtmlElement& element) {
    
    LOG_INFO(L"PdfConverter::processHtml - START");
    processGenericBlock(element); // Singurul apel rămas
    LOG_INFO(L"PdfConverter::processHtml - END");
}

void PdfConverter::processDiv(const XhtmlElement& element) {
    
    LOG_INFO(L"PdfConverter::processDiv - START");
    processGenericBlock(element);
    LOG_INFO(L"PdfConverter::processDiv - END");
}

void PdfConverter::processP(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processP - START");
    processGenericBlock(element); 
    LOG_INFO(L"PdfConverter::processP - END");
}

void PdfConverter::processB(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processB - START");
    processGenericInline(element);
    LOG_INFO(L"PdfConverter::processb - END");
}

void PdfConverter::processI(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processB - START");
    processGenericInline(element);
    LOG_INFO(L"PdfConverter::processb - END");
}

void PdfConverter::processSpan(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processSpan - START");
    processGenericInline(element);
    LOG_INFO(L"PdfConverter::processSpan - END");
}



void PdfConverter::processH1(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processH1 - START");
    processGenericInline(element);
    LOG_INFO(L"PdfConverter::processH1 - END");
}

/*
void PdfConverter::processImg(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processImg - START");
    LOG_INFO(L"PdfConverter::processImg - END");
}
*/
/*
void PdfConverter::processSpan(const XhtmlElement& element) {
    
    LOG_INFO(L"PdfConverter::processSpan - START");

    for (const auto& child : element.subElements) {
        processNodeRecursive(child);
    }

    LOG_INFO(L"PdfConverter::processSpan - END");
}
*/

void PdfConverter::processBR(const XhtmlElement& element) {
    LOG_INFO(L"PdfConverter::processBR - START");
    if(m_lineBuffer.empty())
        processLineBreak();
    else
        newLineAndCheckPageBreak();
    
    LOG_INFO(L"PdfConverter::processSpan - END");
}


void PdfConverter::processGenericInline(const XhtmlElement& element) {
    
    LOG_INFO(L"PdfConverter::processGenericInline - START");

    // 1. Salvează contextul părinte (block) pe stivă
    m_contextStack.push(m_context);

    // 2. Aplică stilul (m_context.style este deja actualizat de processNodeRecursive)
    // 💡 NOTĂ: Dacă ați eliminat applyCssToContext din processNodeRecursive, adăugați-l aici.
    m_context.current_xhtml_element = &element;
    applyCssToContext(element);

    if (element.getTagName() == L"b") {
        m_context.style.fontWeight = L"bold";
    }
    else if (element.getTagName() == L"i") {
        m_context.style.fontStyle = L"italic";
    }
    else if (element.getTagName() == L"h1") {
        m_context.style.fontSize *= 2.; // De exemplu, dublați fontul
        m_context.style.fontWeight = L"bold";

        // Adăugați margini implicite (dacă nu sunt suprascrise de CSS)
        // Set margin top/bottom la 0.67em (sau o valoare fixă rezonabilă)
        // Presupunând că 1em = 12pt (dimensiunea fontului de bază)
        // 0.67 * 12pt = 8.04pt. Setăm 10pt pentru simplitate.
        // 
        m_context.style.boxModel.marginTop += 10.0;
        m_context.style.boxModel.marginBottom += 10.0;
    }
    else if (element.getTagName() == L"span") {
        //m_context.style.fontStyle = L"italic";
        //applyCssToContext(element);
    }

    for (const auto& child : element.subElements) {
        processNodeRecursive(child);
    }

    // 3. Restaurarea contextului (obligatoriu pentru inline)
    m_context = m_contextStack.top();
    m_contextStack.pop();

    LOG_INFO(L"PdfConverter::processGenericInline - END");
}



void PdfConverter::processNodeRecursive(const XhtmlElement& element) {
    const std::wstring& tagName = element.getTagName();
     

    // 1. Aplică Stilul CSS (Întotdeauna necesar)
    //applyCssToContext(element);
    LOG_DEBUG(L"AM APLICAT STILUL LA: " + element.getTagName());
    
    // ⭐ Verificare rapidă pentru elemente ignorate de structura (display: none)
    if (m_context.style.display == L"none") {
        LOG_DEBUG(L"[IGNORE] Tag-ul <" + tagName + L"> ignorat (display: none).");
        return;
    }

    // 2. DISPATCHERUL PE TAG-URI
    if (tagName == L"#text") {
        processTextNode(element);
    }
    else if (tagName == L"html") {
        processHtml(element);
    }
    else if (tagName == L"body") {
        processBody(element);
    }
    else if (tagName == L"div") {
        processDiv(element); // Tratare Block generic
    }
    else if (tagName == L"img") {
        processImg(element); // Tratare Block generic
    }
    else if (tagName == L"p") {
        processP(element); // Tratare Block specializat
    }
    else if (tagName == L"br") {
        processBR(element); // Tratare Block specializat
    }
    else if (tagName == L"b") {
        processB(element); // Tratare Block specializat
    }
    else if (tagName == L"span") {
        processSpan(element); // Tratare Block specializat
    }
    else if (tagName == L"i") {
        processI(element); // Tratare Block specializat
    }
    else if (tagName == L"h1") {
        processH1(element); // Tratare Block specializat
    }
    else if (tagName == L"table") {
        processTable(element); // Tratare Block specializat
    }
    else if (tagName == L"tr" || tagName == L"td" || tagName == L"th") {
        // Elementele de structură a tabelului NU trebuie să fie procesate 
        // de dispatcher-ul general în timpul randării.
        // Ele sunt gestionate exclusiv de processTable.
        // Aici pur și simplu le ignorăm, deoarece nu ar trebui să ajungem aici
        // din exteriorul unui buildTableStructureRecursive.
        LOG_WARNING(L"[TABLE RENDER] Ignorare element structură tabel în dispatcher-ul recursiv: <" + tagName + L">.");
    }/*
    else if (tagName == L"tr") {
        processTableRow(element); // Tratare Block specializat
    }
    else if (element.getTagName() == L"td" || element.getTagName() == L"th") {
        processTableCell(element); // Tratare Block specializat
    }*/
    else if (tagName == L"head" || tagName == L"style" || tagName == L"script" || tagName == L"title") {
        // Ignoră structurile non-vizibile
        LOG_DEBUG(L"[IGNORE] Tag-ul de structură <" + tagName + L"> ignorat.");
    }
    else {
        // Fallback: Dacă nu e specificat, tratăm ca Block (sau conform display-ului CSS)
        // Aici ar trebui să ai o logică default, bazată pe display
        if (m_context.style.display == L"block" || m_context.style.display == L"list-item") {
            processGenericBlock(element);

        }
        else {
            processGenericInline(element);
        }
    }
    
}


void PdfConverter::processGenericBlock(const XhtmlElement& element) {

    // 1. SALVAREA CONTEXTULUI PĂRINTE ȘI CALCULUL METRICELOR INIȚIALE
    // m_context.metrics este setat aici. (height/next_element_y sunt temporare dacă height: auto)
    
    pushBlockContext(element);
    m_context.current_xhtml_element = &element;

    LOG_DEBUG(L"[FLOW_PUSH] Contextul de bloc a fost creat pentru <" + element.getTagName() + L">. Box Y start: " + std::to_wstring(m_context.metrics.y_start));


    // ==========================================================
    // 🎯 PUNCTUL 1: VERIFICARE PAGE BREAK ÎNAINTE DE CONȚINUT
    // ==========================================================

    // Spațiul de sus al elementului: Margine Top + Border Top + Padding Top
    double required_top_decoration_height = m_context.style.boxModel.marginTop +
        m_context.style.boxModel.borderTopWidth +
        m_context.style.boxModel.paddingTop;

    // Y-ul unde se va termina decorul de sus (marginea de conținut Y_content_start)
    // În layout-ul Top-Down, Y-ul scade, deci trebuie să scădem înălțimea din Y_start.
    double y_end_of_top_decoration = m_context.metrics.y_start - required_top_decoration_height;

    // Obiectivul:
    // Dacă y_end_of_top_decoration ajunge sub limita de jos a paginii, declanșăm break-ul.
    // Limita de jos a paginii (Y crește în jos, valoarea e mai mare, dar logic e "jos")
    double page_bottom_limit = m_currentPage.getHeight() - m_currentPage.getMarginBottom(); // Valoare Y mai mică, poziție inferioară.

    // Calculăm înălțimea totală necesară:
    double required_height_total = m_context.metrics.y_start - page_bottom_limit; // Distanța de la Y_start la limita de jos a paginii.

    // Dacă elementul e splittable, verificăm doar decorul de sus (required_top_decoration_height):
    double height_to_check = required_top_decoration_height;

    // Dacă elementul nu se poate sparge (e.g., tabel mic, imagine)
    if (!element.multiPage() && m_context.metrics.height > 0.0) {
        // Dacă are o înălțime fixă, trebuie să verificăm înălțimea totală a box-ului
        height_to_check = m_context.style.boxModel.marginTop + m_context.metrics.height;
    }

    // APELUL DE VERIFICARE:
    //checkPageBreak(height_to_check);

    // Notă: Dacă newPage() este apelată, m_context.metrics.y_start devine invalid, dar 
    // m_context.cursor_y a fost resetat de newPage() la limita de sus a noii pagini
    // și a fost mutat sub decorul de început re-randat.



    // 2. PROCESAREA COPIILOR (Aceasta avansează m_context.cursor_y)
    for (const auto& child : element.subElements) {
        processNodeRecursive(child);
    }

    // 3. FINALIZARE LINIE (Dacă ultimul conținut inline nu a fost închis)
    if (!m_lineBuffer.empty()) {
        LOG_DEBUG(L"[FLOW_LINE] Se finalizează linia pentru <" + element.getTagName() + L">. Cursor Y curent: " + std::to_wstring(m_context.cursor_y) + L"CONTINUT:"+element.getTagContent());
        newLineAndCheckPageBreak();
    }

    // Asigură-te că cursor_y nu este sub limita de jos setată de conținutul intern.
    // De asemenea, asigură-te că cursor_y reflectă sfârșitul ultimului element (după MarginBottom).

    // ==========================================================
    // 4. RECALCULAREA BOXMETRICS PENTRU ÎNĂLȚIME AUTO
    // ==========================================================

    double content_height_calculated = 0.0;

    if (m_context.style.height <= 0.0) { // Cazul 2: Înălțime Auto
        LOG_DEBUG(L"   AM INTRAT PE pozitie auto. Recalculez BoxMetrics.");

        // Înălțimea Conținutului este distanța dintre Y-ul de start al conținutului și Y-ul final al cursorului.
        // Valoarea Y scade, deci trebuie să scazi finalul din început.
        content_height_calculated = m_context.metrics.y_content_start - m_context.cursor_y;

        if (content_height_calculated < 0.0) {
            content_height_calculated = 0.0; // Înălțimea minimă e 0.
        }

        // Recalculează BoxMetrics, actualizând înălțimea totală și next_element_y.
        // Reutilizăm funcția de calcul, dar îi dăm înălțimea finală a conținutului.
        // NOTĂ: Dacă înălțimea finală a conținutului este mai mică decât min-height, 
        // trebuie să ajustezi content_height_calculated aici.

        // Obținem datele de flow inițiale (de la părinte) pentru recalculare (nu sunt în contextul curent)
        const RenderingContext& parentContext = m_contextStack.top();

        double availableWidth = parentContext.metrics.content_width;
        double x_flow_start = parentContext.metrics.x_content_start;
        double y_flow_start = parentContext.cursor_y; // Presupunem că y_flow_start este poziția y_end a elementului anterior

        // Folosim valorile inițiale de flow ale contextului PĂRINTE pentru a re-calcula.
        // ATENȚIE: y_flow_start ar trebui să fie de fapt `parentContext.cursor_y` de la momentul push-ului.
        // Pentru a evita erori complexe de flow, presupunem că poți reface apelul inițial.

        // SOLUȚIE MAI SIMPLĂ: Actualizează doar înălțimea și flow-ul direct în metrics.
        m_context.metrics.content_height = content_height_calculated;

        double vertical_chromes = m_context.metrics.height - m_context.metrics.content_height; // Box H - Content H inițial (când H=0)

        m_context.metrics.height = content_height_calculated
            + m_context.style.boxModel.paddingTop + m_context.style.boxModel.paddingBottom
            + m_context.style.boxModel.borderTopWidth + m_context.style.boxModel.borderBottomWidth;

        m_context.metrics.y_end = m_context.metrics.y_start - m_context.metrics.height;
        m_context.metrics.next_element_y = m_context.metrics.y_end - m_context.style.boxModel.marginBottom;

    }
    else {
        LOG_DEBUG(L"   AM INTRAT PE pozitie fixa. Folosesc BoxMetrics calculate.");
        // Cazul 1: Înălțime Fixă. Metrics sunt deja corecte.
        // Calculăm doar content_height_calculated pentru log
        content_height_calculated = m_context.metrics.content_height;
    }

    // ==========================================================
    // 5. RANDAREA BOX-ULUI (Box Model Rendering)
    // ==========================================================

    // Folosim direct m_context.metrics
    RenderInstruction instruction;
    instruction.x = m_context.metrics.x_start;
    instruction.y = m_context.metrics.y_end; // Coordonata Y de jos a bordurii (pentru randarea PDF)
    instruction.width = m_context.metrics.width;
    instruction.height = m_context.metrics.height;
    instruction.element = element;
    instruction.z_order = m_context.depth;
    instruction.style = m_context.style;
    instruction.renderFunction = L"box";

    
    m_currentPage.pushFrontInstruction(instruction);

    // --- LOGUL UTILIZÂND BOXMETRICS ---
    LOG_DEBUG(L"[RENDER BOX] <" + element.getTagName() + L"> Detalii Randare Box:");
    LOG_DEBUG(L"    - COORDONATE START (X, Y): (" + std::to_wstring(m_context.metrics.x_start) + L", " + std::to_wstring(m_context.metrics.y_start) + L")");
    LOG_DEBUG(L"    - DIMENSIUNI TOTALE (L x H Bordura Inclusa): " + std::to_wstring(m_context.metrics.width) + L" x " + std::to_wstring(m_context.metrics.height));
    LOG_DEBUG(L"    - COORDONATE FINAL (X, Y): (" + std::to_wstring(m_context.metrics.x_end) + L", " + std::to_wstring(m_context.metrics.y_end) + L")");
    LOG_DEBUG(L"[CALCUL BOX CONTINUT] ");
    LOG_DEBUG(L"    - COORDONATE CONTINUT START (X, Y): (" + std::to_wstring(m_context.metrics.x_content_start) + L", " + std::to_wstring(m_context.metrics.y_content_start) + L")");
    //LOG_DEBUG(L"    - INALTIME CONȚINUT CALCULAT: " + std::to_wstring(content_height_calculated));
    //LOG_DEBUG(L"    - FLOW END (Y Next Element): " + std::to_wstring(m_context.metrics.next_element_y));

    // 6. RESTAURAREA CONTEXTULUI PĂRINTE (Transferă m_context.metrics.next_element_y)

    double new_flow_y = m_context.metrics.next_element_y; // Salvează flow end

    popBlockContext(new_flow_y); // Modifică popBlockContext să primească new_flow_y

    //popBlockContext();

    LOG_INFO(L"=========== PROCESS BLOCK FRAME END: <" + element.getTagName() + L"> ===========");
}


void PdfConverter::pushBlockContext(const XhtmlElement& element) {
    // 1. Salvează contextul PĂRINTE pe stivă
    bool isRoot = m_contextStack.empty();
    m_contextStack.push(m_context);

    // 2. Aplică stilurile pe contextul CURENT (m_context)
    applyCssToContext(element);

    // ----------------------------------------------------
    // 🎯 INTEGRAREA BOXMETRICS
    // ----------------------------------------------------

    // Obține contextul PĂRINTE din vârful stivei.
    const RenderingContext& parentContext = m_contextStack.top();

    // Dacă este primul element (body sau html), metricele se bazează pe pagina curentă
    // Altfel, se bazează pe metricele box-ului PĂRINTE.

    // 3. Determină Lățimea Disponibilă
    // Dacă stiva e goală (body/html), lățimea disponibilă e lățimea paginii.
    // Altfel, este lățimea de conținut a părintelui.
    /*
    double availableWidth = m_contextStack.empty()
        ? m_currentPage.getContentWidth()
        : parentContext.metrics.content_width; // Lățimea conținutului părintelui
    */

    double availableWidth = isRoot // **FOLOSIM VARIABILA SALVATA!**
        ? m_currentPage.getContentWidth()
        : parentContext.metrics.content_width;

    // 4. Determină Punctul de Start al Fluxului
    // Poziția X de start: Începe de la x_content_start al părintelui
    double x_flow_start = isRoot
        ? m_currentPage.getMarginLeft() // Presupunând că m_currentPage are margin_x
        : parentContext.metrics.x_content_start;

    // Poziția Y de start: Continuă de la cursorul Y al părintelui
    double y_flow_start = parentContext.cursor_y; // y-ul unde s-a terminat randarea elementului anterior

    // 5. Calculează BoxMetrics și Salvează în Contextul CURENT (m_context)
    m_context.metrics = calculateBoxMetrics(
        availableWidth,
        x_flow_start,
        y_flow_start,
        0.0 // Înălțimea calculată a conținutului (folosită doar pentru height: auto)
    );

    if (element.multiPage()) {
        ElementContext newContext;
        newContext.element = element; // Sau o referință/pointer la element
        newContext.style = m_context.style;
        // Asigurati-va ca BoxMetrics este calculat corect AICI
        newContext.metric = m_context.metrics;

        m_activeBlockStack.push(newContext);
    }

    // 6. Resetarea Cursorilor PENTRU CONȚINUTUL INTERN
    // Cursorul X trebuie să înceapă de la limita de conținut a box-ului curent
    m_context.cursor_x = m_context.metrics.x_content_start;

    // Cursorul Y trebuie să înceapă de la limita Y de conținut a box-ului curent
    m_context.cursor_y = m_context.metrics.y_content_start;

    // Limita de lățime (dreapta) a fluxului devine lățimea de conținut a box-ului curent
    //m_context.flow_width_limit = m_context.metrics.content_width;

    // Marginea X (limita stângă a flow-ului) devine de asemenea x_content_start
    //m_context.margin_x = m_context.metrics.x_content_start;
    
    //m_context.current_line_start_x = m_context.metrics.x_content_start;
}


BoxMetrics PdfConverter::calculateBoxMetrics(
    double availableWidth,
    double x_flow_start,
    double y_flow_start,
    double content_height_calculated = 0.0 // Folosit pentru 'height: auto' SAU înălțimea reală consumată de copii
) {


    LOG_DEBUG(L"START CALCUL METRICS:");
    LOG_DEBUG(L"     availableWidth:" + std::to_wstring(availableWidth));
    LOG_DEBUG(L"     x_flow_start:" + std::to_wstring(x_flow_start));
    LOG_DEBUG(L"     y_flow_start:" + std::to_wstring(y_flow_start));
    LOG_DEBUG(L"     content_height_calculated:" + std::to_wstring(content_height_calculated));
    LOG_DEBUG(L"----------------------------");

    BoxMetrics metrics = {};
    const BoxModel& bm = m_context.style.boxModel;

    // ==========================================================
    // PASUL 1: CALCUL DIMENSIUNILOR FINALE (Width & Height)
    // ==========================================================

    // --- 1.1 Lățimea (Width) ---
    double horizontal_chromes = bm.paddingLeft + bm.paddingRight +
        bm.borderLeftWidth + bm.borderRightWidth;

    if (m_context.style.width > 0.0) {
        // Lățime fixa: umple spațiul disponibil
        metrics.width = m_context.style.width;
    }
    else {
        // Lățime auto: umple spațiul disponibil
        metrics.width = availableWidth - bm.marginLeft - bm.marginRight;
    }

    // --- 1.2 Înălțimea (Height) ---

    double vertical_chromes = bm.paddingTop + bm.paddingBottom +
        bm.borderTopWidth + bm.borderBottomWidth;

    if (m_context.style.height > 0.0) {
        // Cazul A: Înălțime fixă (Style.height)

        // ⚠️ LOGICA NOUĂ PENTRU A PREVENI TRUNCHIEREA FLOW-ULUI ⚠️
        // Comparăm înălțimea fixă a conținutului cu înălțimea reală consumată de copii (content_height_calculated).

        // 1. Înălțimea conținutului cerută de CSS (din H_fix - chromes)
        double content_required_by_style = m_context.style.height - vertical_chromes;

        // 2. Înălțimea efectivă a conținutului trebuie să fie cel puțin înălțimea cerută
        // de stil SAU înălțimea consumată de copii (max).
        double effective_content_height = std::max<double>(content_required_by_style, content_height_calculated);

        // Înălțimea Box-ului devine Înălțimea EFEECTIVĂ a conținutului + chromes.
        // Acest lucru simulează min-height, lăsând box-ul să se extindă dacă flow-ul îl depășește.
        metrics.height = effective_content_height + vertical_chromes;
    }
    else {
        // Cazul B: Înălțime auto (Înălțimea Box-ului = Conținut + Chromes)
        metrics.height = content_height_calculated + vertical_chromes;
    }


    // ==========================================================
    // PASUL 2: CALCUL COORDONATELOR BOX (Marginea Bordurii)
    // ==========================================================

    metrics.x_start = x_flow_start + bm.marginLeft;
    metrics.y_start = y_flow_start - bm.marginTop;

    //metrics.x_end = metrics.x_start + metrics.width;// -bm.borderLeftWidth - bm.borderRightWidth;
    metrics.x_end = metrics.x_start + metrics.width + bm.borderLeftWidth + bm.borderRightWidth;

    // Y_end se calculează pe baza noii metrics.height ajustate.
    //metrics.y_end = metrics.y_start - metrics.height;// -bm.borderBottomWidth;
    metrics.y_end = metrics.y_start - metrics.height - bm.borderBottomWidth;


    // ==========================================================
    // PASUL 3 & 4: DIMENSIUNILE ȘI COORDONATELE CONȚINUTULUI
    // (Rămân neschimbate)
    // ==========================================================

    metrics.content_width = metrics.width - horizontal_chromes;
    metrics.content_height = metrics.height - vertical_chromes;

    metrics.x_content_start = metrics.x_start + bm.borderLeftWidth + bm.paddingLeft;
    metrics.y_content_start = metrics.y_start - bm.borderTopWidth - bm.paddingTop;


    // ==========================================================
    // PASUL 5: POZIȚIA URMĂTORULUI ELEMENT (Flow End)
    // (Folosește noul Y_end ajustat)
    // ==========================================================

    // Următorul element (în afara acestui Box) începe după marginea inferioară.
    metrics.next_element_y = metrics.y_end - bm.marginBottom;
    metrics.next_element_x = metrics.x_content_start;


    LOG_DEBUG(L"END CALCUL METRICS:");
    LOG_DEBUG(L"     Start x/y:" + std::to_wstring(metrics.x_start) + L"/" + std::to_wstring(metrics.y_start));
    LOG_DEBUG(L"     Width" + std::to_wstring(metrics.width) + L" Height:" + std::to_wstring(metrics.height));
    LOG_DEBUG(L"     End x/y:" + std::to_wstring(metrics.x_end) + L"/" + std::to_wstring(metrics.y_end));

    LOG_DEBUG(L"----------------------------");



    return metrics;
}

void PdfConverter::popBlockContext(double next_element_y_from_child) {
    // 1. Reține poziția Y pentru următorul element (calculată în BoxMetrics)
   // double tmp_y_end = m_context.metrics.y_end; // BoxMetrics din contextul tabelului
    //double tmp_margin_bottom = m_context.style.boxModel.marginBottom;
   // RenderingContext currentChildContext = m_context; // Salvează contextul copilului
    std::size_t finishedElementId = m_context.current_xhtml_element->getTagId();
    // 2. RESTAURAREA Contextului PĂRINTE (care conține pointerul vechi, corect)
    if (m_contextStack.empty()) {
        // Nu ar trebui să se întâmple, dar e o verificare de siguranță.
        return;
    }
    m_context = m_contextStack.top();
    m_contextStack.pop();

    //double final_cursor_y = next_element_y_from_child - tmp_margin_bottom;
    m_context.cursor_y = next_element_y_from_child;
    // Scoate elementul din stivă dacă a fost salvat
    if (!m_activeBlockStack.empty() && m_activeBlockStack.top().element.getTagId() == finishedElementId) {
        m_activeBlockStack.pop();
    }
}








void PdfConverter::processTableRow(const XhtmlElement& element) {

    LOG_INFO(L"PdfConverter::processTr - START");

    //processGenericBlock(element);

    LOG_INFO(L"PdfConverter::processTr - END");
}

void PdfConverter::processTableCell(const XhtmlElement& element) {

    LOG_INFO(L"PdfConverter::processCell - START");

    //processGenericBlock(element);

    LOG_INFO(L"PdfConverter::processCell - END");
}

void PdfConverter::processTable(const XhtmlElement& element) {
    LOG_INFO(L"=========== PROCESS TABLE START: <" + element.getTagName() + L"> ===========");

    // 1. PUSH CONTEXT ȘI PREGĂTIRE METRICS
    pushBlockContext(element);
    applyCssToContext(element);

    auto new_table = std::make_unique<Table>();
    m_context.current_table_ref = new_table.get();
    m_tableStorage.push_back(std::move(new_table));
    //m_context.table_stack.emplace_back();

    //Table& current_table = m_context.table_stack.back();
    Table& current_table = *m_context.current_table_ref;

    const XhtmlElement& table_element = element;
    const Style& table_style = m_context.style; 

    // Resetăm structura logică a tabelului
    current_table.rows.clear();

    // ==========================================================
    // FAZA 1: CONSTRUCȚIA MATRICEI LOGICE (Populare table_data)
    // ==========================================================

    // Aici lansăm parcurgerea recursivă a copiilor (<tr>, <tbody>, etc.)
    current_table.xhtml_element_ref = &element;
    buildTableStructureRecursive(element, current_table);

    int max_cols = 0;
    LOG_DEBUG(L"--- DUMP STRUCTURĂ TABEL LOGIC ---");
    for (size_t i = 0; i < current_table.rows.size(); ++i) {
        const auto& row = current_table.rows[i];
        LOG_DEBUG(L"Rând " + std::to_wstring(i) + L": conține " + std::to_wstring(row.cells.size()) + L" celule.");
        max_cols = std::max<int>(max_cols, (int)row.cells.size());
    }
    LOG_DEBUG(L"Număr maxim de coloane (simplificat): " + std::to_wstring(max_cols));
    current_table.print();
    LOG_DEBUG(L"-----------------------------------");

   //daca nu am celule ies
    if (max_cols == 0) {
        LOG_WARNING(L"[TABLE SIZING] Tabelul nu conține celule. Ieșire timpurie.");
        // Aici trebuie să ieși corect (pop context, avansează Y cu înălțimea boxului table)
        popBlockContext(m_context.metrics.next_element_y);
        return;
    }
  
    calculateTableLayout(current_table);

        // ==========================================================
        // FAZA 3: RANDAREA ȘI CALCULUL ÎNĂLȚIMII
        // ==========================================================

        double current_y = m_context.metrics.y_content_start; // Y-ul de start al primei rând
        double final_table_height = 0.0;
        
        int max_cols_calculated = (int)current_table.column_x_start.size();

        // Dacă layout-ul a eșuat (deși nu ar trebui)
        if (max_cols_calculated == 0) {
            LOG_WARNING(L"[TABLE RENDER] Layout-ul a eșuat sau tabelul este gol după calcul. Ieșire.");
            // Asigură-te că POP-ul contextului se întâmplă oricum:
            popBlockContext(m_context.metrics.next_element_y);

          //  if (!m_context.table_stack.empty()) m_context.table_stack.pop_back();
          //  return;
        }

    for (size_t row_index = 0; row_index < current_table.rows.size(); ++row_index) {
        Row& row_data = current_table.rows[row_index];
        double max_row_height = 0.0;
        int current_col_base_index = 0; // Indexul coloanei de bază (0, 1, 2...)

    // 3a. PROCESAREA CELULELOR
        for (Cell& cell : row_data.cells) {
            if (current_col_base_index >= max_cols_calculated) {
                // Aceasta indică că numărul de celule/colspan-uri depășește numărul de coloane
                // calculate de getMaxColumns() din Faza 1.
                LOG_ERROR(L"[TABLE RENDER ERROR] Indicele de bază al celulei depășește numărul de coloane calculate. Sări peste celulă.");
                current_col_base_index += std::max<int>(1, cell.colspan);
                continue;
            }

            // 1. CALCULUL LĂȚIMII ȘI POZIȚIEI X
            double cell_width = 0.0;
            // Poziția X de start este întotdeauna X-ul de start al primei coloane pe care o ocupă celula
            double cell_x_start = current_table.column_x_start[current_col_base_index];

            int actual_colspan = std::max<int>(1, cell.colspan);

            // Sumăm lățimile coloanelor acoperite de colspan
            for (int j = 0; j < actual_colspan; ++j) {
                int col_index_to_add = current_col_base_index + j;

                if (col_index_to_add < max_cols_calculated) {
                    cell_width += current_table.column_widths[col_index_to_add];
                }
                else {
                    // Dacă colspan-ul depășește, ieșim din buclă, dar nu e o eroare fatală
                    LOG_WARNING(L"[TABLE RENDER] Colspan depășește limitele tabelului.");
                    break;
                }
            }

            // Verificare de siguranță: dacă lățimea nu s-a calculat, o luăm pe cea a coloanei unice.
            if (cell_width <= 0.0) {
                cell_width = current_table.column_widths[current_col_base_index];
            }

            // 2. APLICAREA STILULUI ȘI CALCULUL ÎNĂLȚIMII

            const XhtmlElement* cell_element = cell.xhtml_element_ref;
            /*
            if (cell_element == nullptr) {
                LOG_ERROR(L"[TABLE RENDER] EROARE CRITICĂ: Referința la elementul celulei este NULL. Sări peste această celulă.");
                current_col_base_index += std::max<int>(1, cell.colspan); // Asigură-te că avansezi indexul coloanei
                continue; // Treci la următoarea celulă
            }
            LOG_ERROR(L"ELEMENT:" + cell_element->getTagName()+to_wstring<int>(cell_element->getTagId()));
            */
            if (cell_element == nullptr || cell_element->getTagId() != cell.internal_element_id) {
                LOG_WARNING(L"[TABLE FIX] Referința directă la celulă invalidată. Se încearcă regăsirea prin ID.");

                // 🛑 AICI intră logica de recuperare prin ID
                // Trebuie să ai o funcție (ex: findNodeById) care parcurge DOM-ul persistent și returnează XhtmlElement*
                cell_element = m_xhtml.getElementByInternalId(cell.internal_element_id);
            }
            // Aplicăm CSS-ul celulei peste stilul moștenit pentru a obține padding/border/culoare
            // **IMPORTANT: Lățimea 'width' din acest stil va fi ignorată în favoarea 'cell_width' calculat.**
            Style current_cell_style = m_context.style;
            applyCssToStyle(*cell_element, current_cell_style); // Folosește *cell_element

            // Obținem înălțimea totală a rândului (din Faza 2B)
            // Presupunem că row_heights a fost populat și are un index corespunzător.
            // Înălțimea rândului (row_height_css) ar trebui să fie deja stocată în table_data.row_heights
            // (Aici folosesc max_row_height temporar până la implementarea logicii de row_y_start)
            double row_height_css = current_table.row_heights[row_index]; // Presupunem că 'row_index' este disponibil
            double cell_total_height = row_height_css;

            // 3. POPULAREA INSTRUCTIUNII DE RANDARE (BOX)

            // În acest moment, 'cell_width' este Lățimea Box-ului (inclusiv border/padding)
            // Coordonatele celulei (Box-ul extern)


            //current_cell_style.backgroundColor = { -1.0, -1.0, -1.0 };

            RenderInstruction instruction;
            instruction.x = cell_x_start;
            // Y-ul box-ului (coordonata de jos a bordurii)
            instruction.y = current_y - cell_total_height;

            instruction.width = cell_width;
            instruction.height = cell_total_height;
            instruction.element = *cell_element;
            instruction.z_order = m_context.depth;
            // Trecem stilul aplicat, care include border/padding/background/text-color
            instruction.style = current_cell_style;
            instruction.renderFunction = L"box";

            
            m_currentPage.pushFrontInstruction(instruction);
            

            // ==========================================================
            // 4. RANDAREA CONȚINUTULUI CELULEI (Simplificată cu ContextGuard)
            // ==========================================================

            // a. Calculul dimensiunilor de conținut (Inner Box)
            double horizontal_decoration =
                current_cell_style.boxModel.paddingLeft +
                current_cell_style.boxModel.paddingRight +
                current_cell_style.boxModel.borderLeftWidth +
                current_cell_style.boxModel.borderRightWidth;

            double cell_inner_width = cell_width - horizontal_decoration;
            double cell_inner_x = cell_x_start + current_cell_style.boxModel.borderLeftWidth + current_cell_style.boxModel.paddingLeft;
            double cell_inner_y_start = current_y - current_cell_style.boxModel.borderTopWidth - current_cell_style.boxModel.paddingTop;
            double vertical_decoration =
                current_cell_style.boxModel.paddingTop +
                current_cell_style.boxModel.paddingBottom +
                current_cell_style.boxModel.borderTopWidth +
                current_cell_style.boxModel.borderBottomWidth;

            double flow_max_height = cell_total_height - vertical_decoration;


            current_cell_style.width = cell_width;
            
            m_context.style = current_cell_style;
            // --------------------------------------------------------------------
            // 💡 PAS 1: SALVEAZĂ CONTEXTUL CURENT (Tabelul) AUTOMAT
            // --------------------------------------------------------------------
            // La intrarea în acest bloc, contextul curent este salvat în 'guard'.
            ContextGuard guard(m_context);


            // --------------------------------------------------------------------
            // 💡 PAS 2: SETAREA CONTEXTULUI CELULEI (Modificarea stării)
            // --------------------------------------------------------------------

            m_context.current_xhtml_element = cell_element;
            m_context.depth++;
            //m_context.inside_table_cell = true;
            m_context.style = current_cell_style;

            // Setează limitele de Flow
            //m_context.margin_x = cell_inner_x;
            m_context.metrics.x_content_start = cell_inner_x;
            m_context.cursor_x = cell_inner_x;
            //m_context.current_line_start_x = cell_inner_x;
            m_context.metrics.x_content_start = cell_inner_x;

            // Setează poziția de start Y
            m_context.cursor_y = cell_inner_y_start;
            //m_context.content_start_y = cell_inner_y_start;

            // Setează Lățimea Flow-ului
            //m_context.flow_width_limit = cell_inner_width;
            m_context.metrics.content_width = cell_inner_width;
            

            // Resetarea stării liniei
           // m_context.current_line_has_content = false;
            //m_context.last_item_was_space = false;

           BoxMetrics cell_box_metrics = calculateBoxMetrics(
                cell_inner_width, // Lățimea reală disponibilă pentru flow
                cell_inner_x,     // Poziția X de start pentru flow (după margini/padding părinte)
                cell_inner_y_start, // Poziția Y de start a box-ului
                0.0 // Înălțimea nu este cunoscută încă, dar setăm 0.0
            );
            m_context.metrics = cell_box_metrics;

            m_context.cursor_x = m_context.metrics.x_content_start;
            //m_context.current_line_start_x = m_context.metrics.x_content_start;
            // --------------------------------------------------------------------
            // 💡 PAS 3: PROCESAREA CONȚINUTULUI
            // --------------------------------------------------------------------
            for (const auto& child : cell_element->subElements) {
                processNodeRecursive(child);
            }

            if (!m_lineBuffer.empty()) {
                LOG_DEBUG(L"[CELL FLOW] Fortare finalizare linie la iesirea din celula.");
                newLineAndCheckPageBreak();
            }
            // --------------------------------------------------------------------
            // 💡 PAS 4: RESTAURARE AUTOMATĂ
            // --------------------------------------------------------------------
            // Când se iese din scope-ul acestui bloc (unde s-a definit 'guard'), 
            // destructorul ContextGuard este apelat, restaurând m_context la starea salvată.
           
            // 4. PREGĂTIRE PENTRU URMĂTOAREA CELULĂ
            max_row_height = std::max<double>(max_row_height, cell_total_height);
            
            current_col_base_index += actual_colspan;
        }

        // 3b. FINALIZARE RÂND
        double row_height = max_row_height;

        // Randăm Box-ul Rândului (dacă are background/border)
        // TODO: Aici s-ar adăuga o instrucțiune de randare pentru background/border-ul rândului.
       
        // Avansăm Y pentru rândul următor
        current_y -= row_height;
        final_table_height += row_height;
    }

    // 4. FINALIZARE TABLE BOX
    // ... (Logica de finalizare a tabelului din procesul anterior)
    m_context.cursor_y = current_y;
    // CALCUL FINAL ÎNĂLȚIME BOX TABLE
    double box_y_start = m_context.metrics.y_start; // Y-ul de sus al boxului (443.000)

    // Adaugă marginile/padding-urile de sus ale tabelului înapoi
    double table_content_top_y = m_context.metrics.y_content_start;
    double top_offset = box_y_start - table_content_top_y; // Margin/Border/Padding Top

    // Inaltimea totala a box-ului (inclusiv margin, border, padding)
    double final_box_height = top_offset + final_table_height + m_context.style.boxModel.paddingBottom + m_context.style.boxModel.borderBottomWidth + m_context.style.boxModel.marginBottom;

    // Setează BoxMetrics-urile finale ale tabelului
    m_context.metrics.height = final_box_height;
    m_context.metrics.y_end = box_y_start - final_box_height; // Noul Y final al box-ului (Y-ul de sub marginea de jos)
  
    popBlockContext(m_context.metrics.y_end);

//    if (!m_context.table_stack.empty()) {
//        m_context.table_stack.pop_back();
//    }

    LOG_INFO(L"=========== PROCESS TABLE FRAME END: <" + element.getTagName() + L"> ===========");
}

void PdfConverter::buildTableStructureRecursive(const XhtmlElement& parent, Table& table_data) {
    for (const auto& child : parent.subElements) {
        std::wstring tag = child.getTagName();

        if (tag == L"tbody" || tag == L"thead" || tag == L"tfoot") {
            // Dacă întâlnim un grup de rânduri, intrăm recursiv în el
            buildTableStructureRecursive(child, table_data);
        }
        else if (tag == L"tr") {
            Row new_row;
            new_row.xhtml_element_ref = &child;
            new_row.internal_element_id = child.getTagId();
            for (const auto& cell_element : child.subElements) {
                if (cell_element.getTagName() == L"td" || cell_element.getTagName() == L"th") {
                    Cell new_cell;
                    // TODO: Aici se va face citirea colspan/rowspan din atribute
                    std::wstring c_span = cell_element.getAttribute(L"colspan");
                    std::wstring r_span = cell_element.getAttribute(L"rowspan");
                    new_cell.colspan = c_span == L"" ? 1 : from_wstring<int>(c_span);
                    new_cell.rowspan = r_span == L"" ? 1 : from_wstring<int>(r_span);

                    // Salvarea referintei/datelor XhtmlElement (Optional, dar util pentru randare)
                    new_cell.xhtml_element_ref = &cell_element; 
                    new_cell.internal_element_id = cell_element.getTagId();

                    new_row.cells.push_back(new_cell);
                }
            }
            // Adaugam rândul complet populat la structura tabelului
            table_data.rows.push_back(new_row);
        }
        // Ignorăm alte elemente (ex: caption) deocamdată
    }
}


void PdfConverter::calculateTableLayout(Table& table_data) {
    int max_cols = table_data.getMaxColumns();
    if (max_cols == 0) {
        LOG_WARNING(L"[LAYOUT] Tabelul nu conține coloane. Ieșire timpurie.");
        return;
    }

    double table_content_width = m_context.metrics.content_width;

    // --- FAZA 2A: CALCULUL LĂȚIMILOR COLOANELOR (Citire CSS și Distribuție) ---
    table_data.column_widths.assign(max_cols, 0.0);
    double total_fixed_width = 0.0;

    // Presupunem că prima linie (dacă există) definește lățimile implicite ale coloanelor
    if (!table_data.rows.empty()) {
        const Row& first_row = table_data.rows[0];
        int current_col_base_index = 0;
        // Citim lățimile (width) din CSS-ul aplicat primelor celule

        // Iterăm prin celulele RÂNDULUI (Nu prin indexul coloanei)
        for (const auto& cell : first_row.cells) {

            // Sărim peste coloanele deja ocupate, dacă este cazul (nu este cazul in prima faza)
            // Aici nu este necesar, deoarece ne bazăm pe indexul de bază al celulei în rând.

            const XhtmlElement* cell_element = cell.xhtml_element_ref;
            Style temp_style = m_context.style;
            temp_style.width = -1.0;
            applyCssToStyle(*cell_element, temp_style);

            double col_width_css = temp_style.width;
            //LOG_ERROR(L"am gasit latime:" + temp_style.ruleName + to_wstring<double>(col_width_css));
            // 1. Daca lățimea este setată explicit ( > 0)
            if (col_width_css > 0.0) {
                int colspan = cell.colspan;
                double width_per_col = col_width_css / colspan;

                for (int j = 0; j < colspan; ++j) {
                    int col_index = current_col_base_index + j;

                    // Verificare de siguranță: asigură-te că nu ieșim din max_cols
                    // și că nu suprascriem o lățime deja definită (deși în Faza 1 nu ar trebui)
                    if (col_index < max_cols && table_data.column_widths[col_index] == 0.0) {
                        table_data.column_widths[col_index] = width_per_col;
                        total_fixed_width += width_per_col;
                    }
                }

            }
            current_col_base_index += cell.colspan;
            if (current_col_base_index >= max_cols) break;
        }
    }
    // Distribuie spațiul rămas către coloanele care nu au avut lățime fixă (lățime 0.0)
    double remaining_width = table_content_width - total_fixed_width;
    int auto_cols_count = (int)std::count(table_data.column_widths.begin(), table_data.column_widths.end(), 0.0);
    double distributed_width = (auto_cols_count > 0 && remaining_width > 0) ? (remaining_width / auto_cols_count) : 0.0;

    // Finalizează lățimile coloanelor și calculează pozițiile X
    double current_x = m_context.metrics.x_content_start;
    table_data.column_x_start.clear();

    for (int i = 0; i < max_cols; ++i) {
        if (table_data.column_widths[i] == 0.0) {
            table_data.column_widths[i] = distributed_width;
        }
        table_data.column_x_start.push_back(current_x);
        current_x += table_data.column_widths[i];
    }

    // --- FAZA 2B: CALCULUL ÎNĂLȚIMILOR RÂNDURILOR (Citire CSS) ---
    table_data.row_heights.clear();
    table_data.row_y_start.clear();

    // Calculul y_start se face în Faza 3, dar citim height-ul acum.
    for (const auto& row_data : table_data.rows) {
        Style row_style = m_context.style;
        applyCssToStyle(*row_data.xhtml_element_ref, row_style);

        // Înălțimea rândului citită direct din CSS
        double row_height_css = row_style.height;

        // Presupunând că ai default-uri (de ex. 18pt) dacă nu e setată
        if (row_height_css <= 0.0) {
            row_height_css = 18.0; // Valoare implicită (sau o valoare calculată dintr-un font-size implicit)
        }

        table_data.row_heights.push_back(row_height_css);
    }
    LOG_DEBUG(L"[LAYOUT_FINAL] Latimi Coloane:");
    for (int i = 0; i < max_cols; ++i) {
        LOG_DEBUG(L"Col [" + std::to_wstring(i) + L"]: Width=" + std::to_wstring(table_data.column_widths[i]) +
            L", X Start=" + std::to_wstring(table_data.column_x_start[i]));
    }
    LOG_DEBUG(L"[LAYOUT] Lățimile coloanelor și înălțimile rândurilor CSS au fost calculate și stocate.");
}


void PdfConverter::newPage() {

   
    LOG_INFO(L"=============== INCEPE NEW PAGE: " + std::to_wstring(m_currentPage.getPageNumber() + 1) + L" ===============");
    paintCurentPage();
    // 1. Salvează instrucțiunile colectate (m_renderQueue) pe pagina curentă
    // Presupunând că 'Page' este acum gestionată într-un vector de pagini
    // În acest exemplu, vom presupune o listă de pagini (m_documentPages)

    // Adaugă pagina curentă finalizată în lista globală de pagini, dacă nu a fost deja adăugată
    if (m_documentPages.empty() || m_documentPages.back().getPageNumber() != m_currentPage.getPageNumber()) {
        m_documentPages.push_back(m_currentPage);
    }
    else {
        // Dacă pagina există deja (dar nu e finalizată), o actualizăm.
        m_documentPages.back() = m_currentPage;
    }

    // 2. Crearea Noii Pagini
    int nextNumber = m_documentPages.size() + 1;
    // Preluam setarile de dimensiune/margine de la pagina anterioara
    m_currentPage = Page(m_currentPage.getSizeName(), m_currentPage.getOrientation(), m_currentPage.getMarginTop(), m_currentPage.getMarginRight(), m_currentPage.getMarginBottom(), m_currentPage.getMarginLeft(), nextNumber);
    
    
    // 3. Resetarea stării de randare
    //m_renderQueue.clear();
    m_currentPage.clearInstrucions();

    //m_pdfWriter.beginPage(m_currentPage);
    // 4. Resetarea Cursorului Y (la limita de sus a conținutului)
    // Cursorul se mută la poziția Y de start pentru conținutul paginii.
    m_context.cursor_y = m_currentPage.getMarginTop();

    // (Aici ar putea fi adăugată randarea Antetului/Subsolului, dacă există)

    // 5. RE-RANDAREA DECORULUI PENTRU ELEMENTELE ACTIVE

    // Vom folosi un Y_limit care se actualizează pe măsură ce adăugăm decor
    double current_y_start = m_context.cursor_y;

    // Iterăm stiva de la bază (body) spre vârf (cel mai interior).
    // Folosim o copie inversată sau un std::vector temporar pentru a itera în ordine corectă (de la exterior la interior).
    std::vector<ElementContext> reversedStack;
    std::stack<ElementContext> tempStack = m_activeBlockStack;
    while (!tempStack.empty()) {
        reversedStack.push_back(tempStack.top());
        tempStack.pop();
    }
    std::reverse(reversedStack.begin(), reversedStack.end()); // De la exterior la interior (body -> div)

    for (const auto& active_context : reversedStack) {
        // Presupunând că active_context.style și active_context.metric sunt populate
        const Style& style = active_context.style;
        const BoxMetrics& metrics = active_context.metric;

        // 🎯 Aici se generează Box-ul de decor pe noua pagină
        // Desenăm: Background, Border Top, Padding Top.
        // Bordura de jos și Padding-ul de jos sunt ignorate, deoarece elementul continuă.

        RenderInstruction boxInstruction = createSpanningBoxInstruction(
            metrics.x_start, // Folosim X-ul inițial salvat
            current_y_start, // Începe de la cursorul Y actual
            metrics.width,   // Folosim lățimea totală a box-ului salvată
            style);

        
        m_currentPage.pushBackInstruction(boxInstruction);

        // Actualizăm cursorul Y pentru următorul element (sau pentru conținutul paginii)
        current_y_start += style.boxModel.borderTopWidth + style.boxModel.paddingTop;
    }

    // Actualizăm cursorul Y al contextului global pentru a începe randarea conținutului.
    m_context.cursor_y = current_y_start;

    LOG_INFO(L"Noua Pagina " + std::to_wstring(m_currentPage.getPageNumber()) + L" a început la Y: " + std::to_wstring(m_context.cursor_y));
}


void PdfConverter::checkPageBreak(double required_height) {
    // Coordonata Y la care se află cursorul curent (cea mai de jos poziție atinsă până acum).
    double current_y = m_context.cursor_y;

    // Limita de jos a paginii (Y mai mic, poziție inferioară)
    double page_bottom_limit = m_currentPage.getHeight() - m_currentPage.getMarginBottom();

    // Spațiul rămas: Distanța dintre cursorul curent și limita de jos a paginii.
    // Deoarece Y crește în sus (valori mai mici = jos), diferența e: current_y - page_bottom_limit
    double remaining_height = current_y - page_bottom_limit;

    // Dacă înălțimea necesară este mai mare decât spațiul rămas:
    if (required_height > remaining_height) {
        LOG_INFO(L"[PAGE BREAK] Schimbare de pagină necesară. Rămas: "
            + std::to_wstring(remaining_height) + L" < Necesar: "
            + std::to_wstring(required_height));

        // Dacă se face Page Break, newPage() va:
        // 1. Finaliza pagina veche.
        // 2. Reseta m_context.cursor_y la top.
        // 3. Re-randa decorul elementelor active (body, div).
        // 4. Muta m_context.cursor_y sub decorul re-randat.
        newPage();
    }
}

RenderInstruction PdfConverter::createSpanningBoxInstruction(
    double x_start,
    double y_start,
    double width,
    const Style& style)
{
    RenderInstruction instruction;

    // 1. Calcularea geometriei
    double box_height = style.boxModel.borderTopWidth + style.boxModel.paddingTop;

    // 2. Coordonatele Box-ului de decor
    instruction.x = x_start;
    instruction.y = y_start;
    instruction.width = width;
    instruction.height = box_height;

    // 3. Starea și Contextul
    instruction.z_order = -1; // Sau o valoare mică, asigură că decorul este sub conținut.
    instruction.style = style; // Copiem stilul complet
    instruction.text_content = L"";

    // 4. Funcția de Randare Specializată
    // Aceasta este esențială. La momentul desenării PDF, motorul va trebui să știe că
    // această instrucțiune este un box care CONTINUĂ, deci nu are bordură de jos.
    instruction.renderFunction = L"SpanningBoxDecoration";

    // NOTA: Deoarece am copiat 'style' complet, funcția de randare 'SpanningBoxDecoration'
    // va trebui să ignore borderBottom, paddingBottom și marginile inferioare.

    return instruction;
}


void PdfConverter::paintCurentPage() {
    // 1. Randarea Fundalurilor și Box-urilor (Painter's Algorithm)
    //for (auto page : m_documentPages)
        for (const auto& instruction : m_currentPage.getRenderQueue()) {
            if (instruction.renderFunction == L"box") {
                // Apelezi direct funcția de desenare de nivel jos.
                // Fără logica complexă de Box Model.
                LOG_INFO(L"DESENEZ:<" + instruction.element.getTagName() + L"> cu coordonatele:X=" +
                    std::to_wstring(instruction.x) + L" Y=" + std::to_wstring(instruction.y) +
                    L" width=" + std::to_wstring(instruction.width) + L" height=" + std::to_wstring(instruction.height) +
                    L" Final X=" + std::to_wstring(instruction.x + instruction.width) + L" Final Y=" + std::to_wstring(instruction.y + instruction.height));
                //LOG_INFO(L"Page start:" + to_wstring<int>(instruction.element.layout_start_page_index) + L" Page end : "+ to_wstring<int>(instruction.element.layout_end_page_index));
                //instruction.style.backgroundColor.print();

                //m_pdfWriter.renderInstruction(instruction); // Adaptezi renderBox să preia structura
                if (instruction.style.borderStyle == L"dashed")
                    m_pdfWriter.addRectangleDashedBorder(
                        instruction.x,
                        instruction.y,
                        instruction.width,
                        instruction.height,
                        instruction.style.backgroundColor,
                        instruction.style.boxModel.borderTopWidth,
                        instruction.style.borderColor
                    );

                if (instruction.style.borderStyle == L"solid")
                    m_pdfWriter.addRectangleSolidBorder(
                        instruction.x,
                        instruction.y,
                        instruction.width,
                        instruction.height,
                        instruction.style.backgroundColor,
                        instruction.style.boxModel.borderTopWidth,
                        instruction.style.borderColor
                    );
            }
            else if (instruction.renderFunction == L"text") {
            //    LOG_ERROR(L"DESENEZ textul: \"" + instruction.text_content + L"\" cu coordonatele:X=" +
            //        std::to_wstring(instruction.x) + L" Y=" + std::to_wstring(instruction.y) +
            //        L" width=" + std::to_wstring(instruction.width) + L" height=" + std::to_wstring(instruction.height) +
            //        L" Final X=" + std::to_wstring(instruction.x + instruction.width) + L" Final Y=" + std::to_wstring(instruction.y + instruction.height));
            // 
                //LOG_INFO(L"Page start:" + to_wstring<int>(instruction.element.layout_start_page_index) + L" Page end : "+ to_wstring<int>(instruction.element.layout_end_page_index));
                //LOG_INFO(L"RuleName:" + instruction.style.ruleName);
                //LOG_INFO(L"FontFamily:" + instruction.style.fontFamily);
                //LOG_INFO(L"FontWeight:" + instruction.style.fontWeight);

                m_pdfWriter.addTextWithSyle(instruction.x,
                    instruction.y,
                    instruction.text_content, instruction.style);
            }
        }

    m_currentPage.clearInstrucions();

}


void PdfConverter::paintPage(Page& page) {
    m_pdfWriter.beginPage(m_currentPage);
    for (const auto& instruction : page.getRenderQueue()) {
        if (instruction.renderFunction == L"box") {
            // Apelezi direct funcția de desenare de nivel jos.
            // Fără logica complexă de Box Model.
            LOG_INFO(L"DESENEZ:<" + instruction.element.getTagName() + L"> cu coordonatele:X=" +
                std::to_wstring(instruction.x) + L" Y=" + std::to_wstring(instruction.y) +
                L" width=" + std::to_wstring(instruction.width) + L" height=" + std::to_wstring(instruction.height) +
                L" Final X=" + std::to_wstring(instruction.x + instruction.width) + L" Final Y=" + std::to_wstring(instruction.y + instruction.height));
            //LOG_INFO(L"Page start:" + to_wstring<int>(instruction.element.layout_start_page_index) + L" Page end : "+ to_wstring<int>(instruction.element.layout_end_page_index));
            //instruction.style.backgroundColor.print();

            //m_pdfWriter.renderInstruction(instruction); // Adaptezi renderBox să preia structura
            if (instruction.style.borderStyle == L"dashed")
                m_pdfWriter.addRectangleDashedBorder(
                    instruction.x,
                    instruction.y,
                    instruction.width,
                    instruction.height,
                    instruction.style.backgroundColor,
                    instruction.style.boxModel.borderTopWidth,
                    instruction.style.borderColor
                );

            if (instruction.style.borderStyle == L"solid")
                m_pdfWriter.addRectangleSolidBorder(
                    instruction.x,
                    instruction.y,
                    instruction.width,
                    instruction.height,
                    instruction.style.backgroundColor,
                    instruction.style.boxModel.borderTopWidth,
                    instruction.style.borderColor
                );
        }
        else if (instruction.renderFunction == L"text") {
            LOG_ERROR(L"DESENEZ textul: \"" + instruction.text_content + L"\" cu coordonatele:X=" +
                std::to_wstring(instruction.x) + L" Y=" + std::to_wstring(instruction.y) +
                L" width=" + std::to_wstring(instruction.width) + L" height=" + std::to_wstring(instruction.height) +
                L" Final X=" + std::to_wstring(instruction.x + instruction.width) + L" Final Y=" + std::to_wstring(instruction.y + instruction.height));
            //LOG_INFO(L"Page start:" + to_wstring<int>(instruction.element.layout_start_page_index) + L" Page end : "+ to_wstring<int>(instruction.element.layout_end_page_index));
            //LOG_INFO(L"RuleName:" + instruction.style.ruleName);
            //LOG_INFO(L"FontFamily:" + instruction.style.fontFamily);
            //LOG_INFO(L"FontWeight:" + instruction.style.fontWeight);

            m_pdfWriter.addTextWithSyle(instruction.x,
                instruction.y,
                instruction.text_content, instruction.style);
        }
        else if (instruction.renderFunction == L"img") {
            //LOG_INFO(L"APEL: m_pdfWriter.addImageFromFile(cale, W:" + std::to_wstring(final_width_pts) + L", H:" + std::to_wstring(final_height_pts) + L", X:" + std::to_wstring(image_x) + L", Y:" + std::to_wstring(image_y) + L")");
            LOG_INFO(L"DESENEZ:<" + instruction.element.getTagName() + L"> cu coordonatele:X=" +
                std::to_wstring(instruction.x) + L" Y=" + std::to_wstring(instruction.y) +
                L" width=" + std::to_wstring(instruction.width) + L" height=" + std::to_wstring(instruction.height) +
                L" Final X=" + std::to_wstring(instruction.x + instruction.width) + L" Final Y=" + std::to_wstring(instruction.y + instruction.height));

            std::string file_path_utf8 = wstring_to_utf8(instruction.text_content);
             m_pdfWriter.addImageFromFile2(
                 file_path_utf8,
                 instruction.width,
                 instruction.height,    // 3. Înălțimea (double)
                 instruction.x,             // 4. Coordonata X (double)
                 instruction.y              // 5. Coordonata Y (double)
            );
            
        }
    }
    m_pdfWriter.endPage();
    m_currentPage.clearInstrucions();


}


void PdfConverter::finalizeAndPaint() {
    // 1. Randarea Fundalurilor și Box-urilor (Painter's Algorithm)
    for (auto page : m_documentPages) {
        paintPage(page);
    }
    paintPage(m_currentPage);

    m_pdfWriter.endPage();
    m_pdfWriter.finalize(); 
      
}
/*
void PdfConverter::processImg(const XhtmlElement& element) {


    //pushBlockContext(element);
    m_context.current_xhtml_element = &element;
    // 1. Aplică Stilizarea Imaginii (m_context.style conține stilul <img>)
    // Acum, Box Model-ul imaginii (margin/padding/border) este disponibil.
    applyCssToContext(element);

    auto src_attr = element.attributes.find(L"src");
    if (src_attr == element.attributes.end() || src_attr->second.empty()) {
        LOG_WARNING(L"<img> tag without src attribute. Skipping.");
       // restoreContextStyle();
        return;
    }
    std::wstring src_value = src_attr->second;

    // 2. Finalizează linia de text precedentă (dacă există)
    if (!m_lineBuffer.empty()) {
        newLineAndCheckPageBreak();
    }

    // Extrage Box Model-ul și dimensiunile de spațiere
    const auto& boxModel = m_context.style.boxModel;
    const double margin_top = boxModel.marginTop;
    const double margin_bottom = boxModel.marginBottom;

    // --- Extrage și convertește dimensiunile finale ale conținutului (Lățime/Înălțime Imagine) ---
    double final_width_pts = 0.0;
    double final_height_pts = 0.0;

    // ... (Logica ta existentă de extragere a lățimii/înălțimii din atribute HTML/CSS) ...
    auto width_attr = element.attributes.find(L"width");
    auto height_attr = element.attributes.find(L"height");

    

    if (width_attr != element.attributes.end()) {
        final_width_pts = ConvertUtils::convertCssLengthToPt(width_attr->second);
    }
    else {
        final_height_pts = m_context.style.width;
    }

    if (height_attr != element.attributes.end()) {
        final_height_pts = ConvertUtils::convertCssLengthToPt(height_attr->second);
    }
    else {
        final_height_pts = m_context.style.height;
    }

    LOG_ERROR(L"ICI:" + width_attr->second + L" " + height_attr->second);

    // Fallback: Utilizează dimensiunile implicite (dacă nu au fost găsite)
    if (final_width_pts <= 0.0) {
        final_width_pts = ConvertUtils::convertCssLengthToPt(L"41.1mm");
    }
    if (final_height_pts <= 0.0) {
        final_height_pts = ConvertUtils::convertCssLengthToPt(L"14.6mm");
    }

    if (final_width_pts <= 0.0 || final_height_pts <= 0.0) {
        LOG_WARNING(L"Image dimensions could not be resolved or are zero. Skipping.");
       // restoreContextStyle();
        return;
    }

    // 3. Calculează Metricele Box-ului pentru <img>
    // Acesta calculează Box-ul imaginii, ținând cont de Box Model și de lățimea părintelui.
    double availableWidth = m_context.metrics.content_width;

    m_context.metrics = calculateBoxMetrics(
        availableWidth,
        m_context.metrics.x_content_start, // X-ul de start al conținutului părintelui
        m_context.cursor_y,
        final_height_pts // Înălțimea CONȚINUTULUI (imaginea)
    );

    // 4. Verificarea Page Break-ului (Atomic Check)
    // Spațiul necesar include Marginea de Sus și Înălțimea Imaginii.
    const double required_height_on_page = margin_top + final_height_pts + margin_bottom; // Inaltimea totala cu margini

    // Verifică dacă spațiul rămas este suficient
    //checkPageBreak(required_height_on_page);

    // 5. Calculul Poziției Y și actualizarea cursorului (Y-Bottom Up)

    // a. Aplica Marja de Sus (Muta cursorul Y în jos)
    double image_y_top_content = m_context.cursor_y - margin_top;

    // b. Calculează Y-ul de bază (colțul stânga jos al imaginii, Y de desenare)
    double image_y_bottom_draw = image_y_top_content - final_height_pts;

    // c. Coordonata X: Poziția de start a conținutului Box-ului (Sub Padding/Border)
    double image_x_draw = m_context.metrics.x_content_start
        + boxModel.paddingLeft
        + boxModel.borderLeftWidth;

    // d. Actualizează cursorul Y pentru următorul element
    // Noul cursor_y este sub Marja de Jos
    m_context.cursor_y = image_y_bottom_draw - margin_bottom;

    // LOGARE DE DIAGNOSTIC:
    std::wstringstream pos_ss;
    pos_ss << L"[IMAGE POS] Desenare la X:" << image_x_draw
        << L", Y (Bottom Draw):" << image_y_bottom_draw
        << L", W:" << final_width_pts
        << L", H:" << final_height_pts
        << L" pt. | Cursor Y ulterior: " << m_context.cursor_y;
    LOG_DEBUG(pos_ss.str());

    // 6. Creează și adaugă instrucțiunea de randare
    RenderInstruction instruction;
    instruction.x = image_x_draw;
    instruction.y = image_y_bottom_draw;
    instruction.width = final_width_pts;
    instruction.height = final_height_pts;
    instruction.style = m_context.style;
    instruction.renderFunction = L"img";
    instruction.z_order = m_context.depth + 1;
    instruction.text_content = src_value; // Calea către imagine
    instruction.element = element;

    //m_currentPage.pushBackInstruction(instruction);
    m_currentPage.pushFrontInstruction(instruction);

    // 7. Finalizare: Resetează cursorul X la marginea stângă a blocului părinte
    m_context.cursor_x = m_context.metrics.x_content_start;

    // 8. Restaurează stilul contextului la cel al părintelui
   // restoreContextStyle();

    double new_flow_y = m_context.metrics.next_element_y; // Salvează flow end

   // popBlockContext(new_flow_y); // Modifică popBlockContext să primească new_flow_y
}
*/


void PdfConverter::clean() {
    while (!m_contextStack.empty()) m_contextStack.pop();
    while (!m_activeBlockStack.empty()) m_activeBlockStack.pop();
    m_lineBuffer.clear();
    

    m_currentPage.clearInstrucions();
    m_documentPages.clear(); // Destructorii Page se apelează, eliberând RenderInstructions.

    // 4. ⚠️ PAS CRITIC: ELIBERAREA RESURSELOR HUMMUS
    // Forțează distrugerea obiectului PDFWriter intern
    m_pdfWriter.hardReset();
}


void PdfConverter::processImg(const XhtmlElement& element) {
    // Salvăm stilul contextului PĂRINTE înainte de a aplica stilul imaginii.
    // Acest lucru asigură că elementele ulterioare (precum <table>) nu moștenesc
    // stilurile CSS specifice imaginii.
    Style parent_style_backup = m_context.style;

    // 1. Aplică Stilizarea Imaginii (m_context.style conține stilul <img>)
    // Aplicăm stilul direct peste contextul curent pentru a accesa BoxModel corect.
    applyCssToContext(element);
    m_context.current_xhtml_element = &element;

    auto src_attr = element.attributes.find(L"src");
    if (src_attr == element.attributes.end() || src_attr->second.empty()) {
        LOG_WARNING(L"<img> tag without src attribute. Skipping.");
        m_context.style = parent_style_backup; // Restaurăm stilul
        return;
    }
    std::wstring src_value = src_attr->second;

    // 2. Finalizează linia de text precedentă (dacă există)
    // Deoarece <img> este un element atomic care consumă spațiu vertical,
    // închidem linia curentă pentru a ne asigura că imaginea începe pe o linie nouă.
    if (!m_lineBuffer.empty()) {
        newLineAndCheckPageBreak();
    }

    // Extrage Box Model-ul și dimensiunile de spațiere
    const auto& boxModel = m_context.style.boxModel;
    const double margin_top = boxModel.marginTop;
    const double margin_bottom = boxModel.marginBottom;

    // --- Extrage și convertește dimensiunile finale ale conținutului (Lățime/Înălțime Imagine) ---
    double final_width_pts = 0.0;
    double final_height_pts = 0.0;

    // ATENȚIE: Aici nu mai putem folosi direct m_context.style.width/height 
    // dacă acestea nu au fost setate explicit ca 'content width/height'.
    // Voi presupune că logica existentă extrage W/H corect:

    auto width_attr = element.attributes.find(L"width");
    auto height_attr = element.attributes.find(L"height");

    if (width_attr != element.attributes.end()) {
        final_width_pts = ConvertUtils::convertCssLengthToPt(width_attr->second);
    }
    // ELSE: Dacă nu există width în atribut, luați din style:
    else if (m_context.style.width > 0.0) {
        final_width_pts = m_context.style.width;
    }

    if (height_attr != element.attributes.end()) {
        final_height_pts = ConvertUtils::convertCssLengthToPt(height_attr->second);
    }
    // ELSE: Dacă nu există height în atribut, luați din style:
    else if (m_context.style.height > 0.0) {
        final_height_pts = m_context.style.height;
    }

    // Fallback: Utilizează dimensiunile implicite (asigurare că nu e 0)
    if (final_width_pts <= 0.0) {
        final_width_pts = ConvertUtils::convertCssLengthToPt(L"41.1mm");
    }
    if (final_height_pts <= 0.0) {
        final_height_pts = ConvertUtils::convertCssLengthToPt(L"14.6mm");
    }

    if (final_width_pts <= 0.0 || final_height_pts <= 0.0) {
        LOG_WARNING(L"Image dimensions could not be resolved or are zero. Skipping.");
        m_context.style = parent_style_backup;
        return;
    }

    // -------------------------------------------------------------------
    // 3. CALCULUL POZIȚIEI ȘI CHECK PAGE BREAK
    // -------------------------------------------------------------------

    // Spațiul vertical total necesar (Margine + Border + Padding + Conținut)
    const double required_height_on_page = margin_top + margin_bottom
        + final_height_pts
        + boxModel.paddingTop + boxModel.paddingBottom
        + boxModel.borderTopWidth + boxModel.borderBottomWidth;

    // Verifică Page Break-ul (Mută pe pagina următoare dacă este necesar)
    // checkPageBreak(required_height_on_page); 

    // 4. Calculul Poziției Y (Y-Bottom Up) și actualizarea cursorului

    // a. Aplica Marja de Sus (Muta cursorul Y în jos)
    double image_y_top_content = m_context.cursor_y - margin_top;

    // b. Calculează Y-ul de bază (colțul stânga jos al imaginii, Y de desenare)
    double image_y_bottom_draw = image_y_top_content - final_height_pts;

    // c. Coordonata X: Poziția de start (Cursorul X curent + Padding/Border stânga)
    // Atenție: Folosim m_context.cursor_x care ar trebui să fie x_content_start după newLineAndCheckPageBreak
    double image_x_draw = m_context.cursor_x
        + boxModel.marginLeft
        + boxModel.borderLeftWidth
        + boxModel.paddingLeft;

    // d. Actualizează cursorul Y pentru următorul element (Sub Marja de Jos)
    m_context.cursor_y = image_y_bottom_draw - margin_bottom;

    // 5. Creează și adaugă instrucțiunea de randare
    RenderInstruction instruction;
    instruction.x = image_x_draw;
    instruction.y = image_y_bottom_draw;
    instruction.width = final_width_pts;
    instruction.height = final_height_pts;
    instruction.style = m_context.style;
    instruction.renderFunction = L"img";
    instruction.z_order = m_context.depth + 1;
    instruction.text_content = src_value; // Calea către imagine
    instruction.element = element;

    m_currentPage.pushFrontInstruction(instruction);

    // 6. Finalizare: Resetarea cursorului X și restaurarea stilului

    // Deoarece am forțat o linie nouă, cursorul X trebuie resetat la start-ul fluxului de conținut
    // al părintelui (m_context.metrics.x_content_start).
    m_context.cursor_x = m_context.metrics.x_content_start;

    // Restaurarea stilului original (cel al <body>)
    m_context.style = parent_style_backup;
}
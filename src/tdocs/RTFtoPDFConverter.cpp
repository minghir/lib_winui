#include "RtfToPdfConverter.hpp"
#include "ConvertUtils.hpp" // Adăugăm utilitarul pentru twipsToPoints
#include <algorithm>
#include <cmath> 

// --- Helper Functions (Definițiile trebuie să fie disponibile) ---

extern std::string wstring_to_utf8(const std::wstring& wstr);
extern std::wstring str_to_wstr(const std::string& str);


// -----------------------------------------------------------------
// METODA PRINCIPALĂ: CONVERTIRE
// -----------------------------------------------------------------

bool RtfToPdfConverter::convert(const std::wstring& filename) {
    // 1. Inițializare document PDF
   
    // Extrage informațiile paginii (în Twips)
    const RtfPage& pageInfoTwips = m_rtfDocument.getPageInfo();

    // ⭐ CONVERSIE: Calculează dimensiunile și marginile în PUNCTE PDF
    pageWidth = ConvertUtils::twipsToPoints(pageInfoTwips.getWidthTwips());
    pageHeight = ConvertUtils::twipsToPoints(pageInfoTwips.getHeightTwips());

    m_marginLeft = ConvertUtils::twipsToPoints(pageInfoTwips.getMarginLeftTwips());
    m_marginRight = ConvertUtils::twipsToPoints(pageInfoTwips.getMarginRightTwips());
    double marginTop = ConvertUtils::twipsToPoints(pageInfoTwips.getMarginTopTwips());

    outputFilePath = filename; // Placeholder
       
    // B. Inițializare state (Moștenite din BasePdfConverter)
    m_pageHeight = pageHeight; // TREBUIE SA FIE INALTIMEA PAGINII (595.35 pt)
    m_contentWidth = pageWidth - m_marginLeft - m_marginRight;
    m_currentY = marginTop;

    if (!m_pdfWriter.initialize(outputFilePath, pageWidth, pageHeight)) {
        LOG_FATAL(L"Eroare fatală la inițializarea PdfWriter. Nu se poate continua.");
        return false;
    }

    RenderInstruction instruction;
    instruction.renderFunction = L"startPage";
    instruction.width = pageWidth;
    instruction.height = pageHeight;
    m_renderQueue.push_back(instruction);

    // C. Pornirea primei pagini
    
    m_currentPageNumber = 1;
    m_totalPagesCount = m_currentPageNumber;

    LOG_SUCCESS(L"Incepe conversia RTF. Dimensiune continut: " + std::to_wstring(m_contentWidth) + L"pt.");

    // 2. Traversarea documentului (Body)
    for (const auto& block : m_rtfDocument.getBlocks()) {
        if (block) { // Verifica pointer valid
            processRtfBlock(*block);
        }
    }
    renderFooter();

    initializeGlobalVarResolvers();

    if (finalizeAndPaint()) {
        LOG_SUCCESS(L"Conversie finalizată cu succes. Total pagini: " + std::to_wstring(m_currentPageNumber));
        return true;
    }
    else {
        LOG_ERROR(L"Eroare la conversia in pdf!");
        return false;
    }
}


void RtfToPdfConverter::newLineAndCheckPageBreak(double requiredHeight) {
    const RtfPage& pageInfoTwips = m_rtfDocument.getPageInfo();
    const double EPSILON = 0.1;

    // Marginile de Jos și de Sus în Puncte
    double marginBottom = ConvertUtils::twipsToPoints(pageInfoTwips.getMarginBottomTwips());
    double marginTop = ConvertUtils::twipsToPoints(pageInfoTwips.getMarginTopTwips());

    // Spațiul rămas între cursorul Y curent și marginea de jos a paginii (exclusiv marginBottom)
    double y_remaining_usable_space = m_pageHeight - m_currentY - marginBottom;

    // Dacă spațiul rămas e mai mic strict decât înălțimea necesară
    if (requiredHeight > y_remaining_usable_space + EPSILON) {
        LOG_INFO(L"[PAGINARE] Pagina plină. Trecere la pagina următoare.");

        // 1. Randare Footer pe pagina curentă (care se va termina)
        // -------------------------------------------------------------
        renderFooter(); // ⭐ APEL AICI
        // -------------------------------------------------------------

        // 2. Finalizează pagina curentă
        RenderInstruction instruction;
        instruction.renderFunction = L"endPage";
        m_renderQueue.push_back(instruction);
        //pdfWriter.endPage();

        // 3. Deschide o pagină nouă 
        pageWidth = ConvertUtils::twipsToPoints(pageInfoTwips.getWidthTwips());
        pageHeight = ConvertUtils::twipsToPoints(pageInfoTwips.getHeightTwips());


        //RenderInstruction instruction;
        instruction.renderFunction = L"startPage";
        instruction.width = pageWidth;
        instruction.height = pageHeight;
        m_renderQueue.push_back(instruction);

        //pdfWriter.startPage(pageWidth, pageHeight);
        m_currentPageNumber++;
        m_totalPagesCount++;

        // 4. Resetează cursorul Y la marginea de sus
        m_currentY = marginTop; // Începe din nou de sus (în PT)

        // 5. Randare Header pe noua pagină (Header-ul este primul element de conținut)
        // -------------------------------------------------------------
        //renderHeader(); // ⭐ APEL HEADER
        // -------------------------------------------------------------
    }
}
// -----------------------------------------------------------------
// 2. FUNCȚII DE RANDARE STRUCTURALĂ
// -----------------------------------------------------------------

/**
 * Traversează un element block (paragraf, tabel, div, etc.).
 */
void RtfToPdfConverter::processRtfBlock(const RtfBlock& block) {

    // ⭐ Notă: RtfBlock este clasa de bază. Trebuie să accesăm Style-ul din obiectele derivate.
    //double marginTop = 5.0; // Placeholder
    //double marginBottom = 5.0; // Placeholder



    // 2. Procesează tipul specific de bloc
    if (const RtfParagraph* paragraph = dynamic_cast<const RtfParagraph*>(&block)) {

       

        processRtfParagraph(*paragraph);

        double marginTop = 2.0; // Placeholder
        m_currentY += marginTop;

    }
    else if (const RtfTable* table = dynamic_cast<const RtfTable*>(&block)) {

       // double marginTop = 2.0; // Placeholder
       //m_currentY += marginTop;
        processRtfTable(*table); // TODO: Implementare Tabel
    }
    // ... alte tipuri de block (liste etc.)

   // m_currentY += marginBottom;
}

// ----------------------------------------------------
// NOU: Funcție Auxiliară pentru Calculul Poziției X de Start a unui Rând
// ----------------------------------------------------

double RtfToPdfConverter::calculateLineXStart(const std::wstring& alignment, double contentWidth, double lineWidth, double marginLeft) const {
    if (alignment == L"center") {
        // Aliniere Centru: marginLeft + (availableSpace / 2)
        // availableSpace = contentWidth - lineWidth
        double availableSpace = contentWidth - lineWidth;
        return marginLeft + (availableSpace / 2.0);
    }
    else if (alignment == L"right") {
        // Aliniere Dreapta: marginLeft + contentWidth - lineWidth
        return marginLeft + contentWidth - lineWidth;
    }
    else {
        // Aliniere Stânga (Implicit): marginLeft
        return marginLeft;
    }
}

// ----------------------------------------------------
// Funcția Principală Modificată: processRtfParagraph
// ----------------------------------------------------

// ----------------------------------------------------
// NOUA Funcție processRtfParagraph
// ----------------------------------------------------

double RtfToPdfConverter::processRtfParagraph(const RtfParagraph& paragraph) {

    double startY = m_currentY; // Salvează poziția Y de start
    const Style& paragraphStyle = paragraph.style;

    std::wstring fontFamily = paragraphStyle.fontFamily;
    if (fontFamily.empty() || fontFamily == L"DefaultFont") {
        fontFamily = L"Arial"; // Folosește Arial ca fallback
    }


    double fontSize = paragraphStyle.fontSize > 0.0 ? paragraphStyle.fontSize : 12.0;

    if (fontSize == 0.0 || fontSize > 20.0) { // Presupunem că > 20.0 pt e o eroare de parsare default (24 half-points)
        fontSize = 12.0; // Fallback la 12pt
    }

    double lineHeightFactor = paragraphStyle.lineHeight > 0.0 ? paragraphStyle.lineHeight : 1.2;

    // Calculul CORECT: înmulțește fontul cu factorul
    double lineHeight = fontSize * lineHeightFactor;

    //LOG_ERROR(L"INALTIME LINEIE CALCULATA:" + std::to_wstring(lineHeight) + L" Si font:" + std::to_wstring(fontSize));
    // 1. Avansare inițială Y și verificare paginare
    //m_currentY += lineHeight;
   // newLineAndCheckPageBreak(lineHeight);

    // 2. Poziția X de start pentru conținut
    double content_x_start = m_marginLeft;// +paragraphStyle.boxModel.paddingLeft;
    double current_x = content_x_start;
    //double y_baseline = m_pageHeight - m_currentY;

    

    // Buffer pentru a acumula lățimea textului pe rândul CURENT (pentru centrare/wrap)
    //std::vector<const RtfSpan*> spansOnCurrentLine;

    // Buffer pentru a acumula textul și stilurile pe rândul CURENT
    std::vector<std::pair<std::wstring, Style>> wordsOnCurrentLine;
    double currentLineContentWidth = 0.0; // Lățimea totală a span-urilor pe rândul curent

    // 3. Iterează și Randează fiecare SPAN
    for (const auto& span : paragraph.spans) {
        const Style& spanStyle = span.style;
        std::vector<std::wstring> words = split_to_words(span.text);
        //print_wstr_vct(words);
        // Iterează prin fiecare CUVÂNT din SPAN
        for (const std::wstring& word : words) {

            // ⭐ A. TRATAREA \page (Hard Page Break) ⭐
            if (word == L"\f") {
                LOG_SUCCESS(L"HARD PAGE BREAK DETECTAT (\\page)");

                // 1. Randare Rând Rămas: Randează orice conținut acumulat înainte de \page
                if (!wordsOnCurrentLine.empty()) {
                    renderWords(wordsOnCurrentLine, paragraphStyle.textAlign, currentLineContentWidth, lineHeight);
                }

                // 2. FORȚAREA SCHIMBĂRII DE PAGINĂ
                // Apelăm newLineAndCheckPageBreak cu o înălțime mare (sau o funcție dedicată)
                // pentru a forța schimbul de pagină.
                // Folosim o valoare suficient de mare pentru a forța întotdeauna un page break.
                newLineAndCheckPageBreak(m_pageHeight);

                // 3. Resetează variabilele (pe noua pagină)
                current_x = content_x_start;
                currentLineContentWidth = 0.0;
                wordsOnCurrentLine.clear();

                continue;
            }


            // ⭐ A. TRATAREA \line (Hard Break) ⭐
            
            //if (word == L"\\n") {
            if (word == L"$line$") {

                // Pasul 1: Randare Rând CURENT (dacă nu e gol)
                if (!wordsOnCurrentLine.empty()) {
                    // Aceasta avansează Y (în interiorul renderWords)
                    //LOG_ERROR(L"ADAUG LINIE NOUA LA PARAGRAF");
                    renderWords(wordsOnCurrentLine, paragraphStyle.textAlign, currentLineContentWidth, lineHeight);
                    //newLineAndCheckPageBreak(lineHeight); // Verifică și schimbă pagina
                    //m_currentY += lineHeight; // Avansare Y #1 (Pentru rândul randat)
                }
                else {
                    // Pasul 1b (NOU): Dacă linia este goală, forțează avansarea Y
                    // Această logică DUPLICĂ avansarea Y din renderWords, dar este crucială aici.
                    //LOG_ERROR(L"ADAUG LINIE NOUA");
                    newLineAndCheckPageBreak(lineHeight);
                    m_currentY += lineHeight;
                }

                // Pasul 2: Resetează variabilele pentru noul rând
                current_x = content_x_start;
                currentLineContentWidth = 0.0;
                wordsOnCurrentLine.clear();

                // Treci la următorul token
                //continue;
            }
            //if (word.length() == 1 && word[0] == L'\t') {
            if (word == L"\\t") {

                // Măsurăm lățimea tabulatorului (36.0 pt)
                double tabWidth = TAB_WIDTH;

                // 🎯 LOGICA DE WORD WRAP pentru TAB
                // Verifică dacă tabulatorul + tot ce e pe rând depășește lățimea conținutului.
                // Tab-ul nu rupe rândul, dar dacă nu încape, rândul anterior trebuie randat.
                if (current_x + tabWidth > m_marginLeft + m_contentWidth) {

                    // Randează conținutul curent și resetează
                    if (!wordsOnCurrentLine.empty()) {
                        renderWords(wordsOnCurrentLine, paragraphStyle.textAlign, currentLineContentWidth, lineHeight);
                    }
                    current_x = content_x_start;
                    currentLineContentWidth = 0.0;
                    wordsOnCurrentLine.clear();
                }

                // 1. Adaugă tab-ul ca element în buffer-ul liniei
                wordsOnCurrentLine.push_back({ L"\\t", spanStyle });

                // 2. Actualizează lățimea liniei
                currentLineContentWidth += tabWidth;
                current_x += tabWidth;

                // Nu mai avem nevoie de `continue`, deoarece `\t` este tratat
                // ca orice alt cuvânt, dar este adăugat la buffer.
                // continue; // Ștergeți acest rând
                continue;
            }
            
            // Măsoară lățimea cuvântului + spațiul (un spațiu după fiecare cuvânt)
            double wordWidth = m_pdfWriter.measureTextWidth(word + L" ", spanStyle);
            // 🎯 LOGICA DE WORD WRAP
            // Verifică dacă cuvântul curent încape pe rândul curent
            
            if (current_x + wordWidth > m_marginLeft + m_contentWidth) {
            
                // A. Finalizează și Randează Rândul CURENT (dacă nu e gol)
                if (!wordsOnCurrentLine.empty()) {
                    renderWords(wordsOnCurrentLine, paragraphStyle.textAlign, currentLineContentWidth, lineHeight);
                }
                // B. Resetează variabilele pentru noul rând
                current_x = content_x_start; // Începe de la stânga
               // m_currentY += lineHeight;
                currentLineContentWidth = 0.0;
                wordsOnCurrentLine.clear();
            }
            // 3. Adaugă Cuvântul CURENT la linia nouă (sau la cea existentă)
            wordsOnCurrentLine.push_back({ word + L" ", spanStyle });
            currentLineContentWidth += wordWidth;
            current_x += wordWidth; // Avansăm cursorul X

        }
    }

    // 4. Randare Rând Rămas (Ultimul rând)
    //if (!wordsOnCurrentLine.empty()) {
    //    renderWords(wordsOnCurrentLine, paragraphStyle.textAlign, currentLineContentWidth, lineHeight);
    //}

    if (!wordsOnCurrentLine.empty()) {
        // Dacă alinierea este justify, forțează alinierea la stânga pentru ultimul rând
        std::wstring finalAlignment = (paragraphStyle.textAlign == L"justify") ? L"left" : paragraphStyle.textAlign;

        renderWords(wordsOnCurrentLine, finalAlignment, currentLineContentWidth, lineHeight);
    }

    double endY = m_currentY;

    LOG_DEBUG(L"Paragraf procesat. Cursor Y mentinut la: " + std::to_wstring(m_currentY));

    return endY - startY; // Returnează înălțimea totală consumată
    
}



void RtfToPdfConverter::renderWords(
    const std::vector<std::pair<std::wstring, Style>>& words,
    const std::wstring& alignment,
    double lineContentWidth,
    double lineHeight)
{
    // Trecere la Rând Nou și Verificare Paginare
    newLineAndCheckPageBreak(lineHeight);
    m_currentY += lineHeight;

    double line_width = m_contentWidth; // Lățimea totală de conținut disponibilă
    double currentX = m_marginLeft;
    double yBaseline = m_pageHeight - m_currentY;

    // Variabile specifice Justify
    bool isJustify = (alignment == L"justify" && words.size() > 1);
    double extraSpacePerGap = 0.0;

    // 1. CALCULUL SPAȚIULUI EXTRA PENTRU JUSTIFY
    if (isJustify) {
        size_t numSpacesToExpand = words.size() - 1;

        if (numSpacesToExpand > 0) {
            // lineContentWidth include lățimea tuturor cuvintelor + lățimea spațiilor originale
            double remainingSpace = line_width - lineContentWidth;

            // Spațiul suplimentar care trebuie adăugat fiecărui spațiu dintre cuvinte
            extraSpacePerGap = remainingSpace / numSpacesToExpand;

            currentX = m_marginLeft; // Pentru Justify, randarea începe întotdeauna de la margine
        }
        else {
            // Dacă există un singur cuvânt, dezactivează Justify și aliniază stânga
            isJustify = false;
        }
    }

    // 2. CALCULUL X START PENTRU NON-JUSTIFY
    if (!isJustify) {
        currentX = calculateLineXStart(
            alignment,
            line_width,
            lineContentWidth,
            m_marginLeft
        );
    }

    // 3. RANDAREA PROPRIU-ZISĂ
    for (size_t i = 0; i < words.size(); ++i) {
        const std::wstring& wordWithSpace = words[i].first;
        const Style& style = words[i].second;
        
        if (wordWithSpace == L"$line$" ||  wordWithSpace == L"$line$ " || wordWithSpace == L" $line$" || wordWithSpace == L" $line$ ") continue;

        if (wordWithSpace == L"\\t") {
            // Nu randăm nimic, doar avansăm cursorul.
            // Presupunând că tabWidth era 36.0 (sau lățimea tab-ului)
            LOG_WARNING(L"AM GASIT TAAAAAAAAAAAAB");
            currentX += TAB_WIDTH;
            continue; // Treci la următorul cuvânt/spațiu
        }
       
        // Separă cuvântul de spațiul de la coadă (asumând un spațiu)
        std::wstring wordOnly;
        if (wordWithSpace.length() > 0 && wordWithSpace.back() == L' ') {
            wordOnly = wordWithSpace.substr(0, wordWithSpace.length() - 1);
        }
        else {
            // Caz de siguranță/eroare: randează tot șirul dacă nu se termină cu spațiu.
            wordOnly = wordWithSpace;
        }
        //std::wstring wordOnly = wordWithSpace.substr(0, wordWithSpace.length() - 1);

        // Calculează lățimea doar a cuvântului
        double wordOnlyWidth = m_pdfWriter.measureTextWidth(wordOnly, style);

        // --- Generează Instrucțiunea de Randare (DOAR CUVÂNTUL) ---
        RenderInstruction instruction;
        instruction.x = currentX;
        instruction.y = yBaseline;
        instruction.text_content = wstr_trim(wordOnly); // Textul final randat este DOAR cuvântul
        instruction.style = style;
        instruction.renderFunction = L"text";
        // Apelați identifyGlobalVars aici dacă este necesar!
        // identifyGlobalVars(wordOnly, instruction.globalVars); 

        m_renderQueue.push_back(instruction);

        // Avansăm cursorul X cu lățimea cuvântului
        currentX += wordOnlyWidth;

        // 4. Avansarea cu spațiul (Spațiul Original + Spațiu Extra)
        if (i < words.size() - 1) { // Aplică spațiu doar între cuvinte

            // Lățimea spațiului original (L" ")
            double originalSpaceWidth = m_pdfWriter.measureTextWidth(L" ", style);

            double totalSpaceAdvance = originalSpaceWidth;

            if (isJustify) {
                // Adaugă spațiul suplimentar (extraSpacePerGap)
                totalSpaceAdvance += extraSpacePerGap;
            }

            currentX += totalSpaceAdvance; // Avansăm cu spațiul ajustat
        }
    }
}



/*
void RtfToPdfConverter::renderWords(
    const std::vector<std::pair<std::wstring, Style>>& words,
    const std::wstring& alignment,
    double lineContentWidth,
    double lineHeight)
{
    // Trecere la Rând Nou și Verificare Paginare
    newLineAndCheckPageBreak(lineHeight);
    m_currentY += lineHeight;
    

    // 1. Calculează poziția X de start pentru aliniere
    double x_start_for_rendering = calculateLineXStart(
        alignment,
        m_contentWidth,
        lineContentWidth, // Lățimea tuturor cuvintelor acumulate
        m_marginLeft // Deja include padding-ul stânga al celulei
    );

    double currentX = x_start_for_rendering;
    double yBaseline = m_pageHeight - m_currentY;

    // 2. Randează fiecare cuvânt
    for (const auto& wordPair : words) {
        const std::wstring& wordWithSpace = wordPair.first;
        
        const Style& style = wordPair.second;

        //if (wordWithSpace == L"\n\n") newLineAndCheckPageBreak(lineHeight);// || wordWithSpace == L"\f") continue;


        RenderInstruction instruction;
        instruction.x = currentX;
        instruction.y = yBaseline;
        instruction.text_content = wordWithSpace;
        instruction.style = style;
        instruction.renderFunction = L"text";
        m_renderQueue.push_back(instruction);

        // Folosește direct addTextWithSyle cu stilul corect al cuvântului
        LOG_INFO(L"RANDEZ:" + wordWithSpace+ L" Font-family:"+style.fontFamily + L" Font-wight:" + style.fontWeight);
        //pdfWriter.addTextWithSyle(currentX, yBaseline, wordWithSpace, style);

        // Măsoară lățimea pentru a avansa cursorul
        double wordWidth = m_pdfWriter.measureTextWidth(wordWithSpace, style);
        currentX += wordWidth;
    }
}
*/

// ----------------------------------------------------
// NOU: Funcție Auxiliară pentru Randarea Span-urilor
// ----------------------------------------------------

void RtfToPdfConverter::renderSpans(const std::vector<const RtfSpan*>& spans, double startX, double yBaseline) {
    double currentX = startX;

    for (const auto* span : spans) {
        // Folosește direct addTextWithSyle cu stilul corect al SPAN-ului
        RenderInstruction instruction;
        instruction.x = currentX;
        instruction.y = yBaseline;
        instruction.text_content = span->text;
        instruction.style = span->style;
        instruction.renderFunction = L"text";
        m_renderQueue.push_back(instruction);

        //pdfWriter.addTextWithSyle(currentX, yBaseline, span->text, span->style);

        // Măsoară lățimea pentru a avansa cursorul
        double spanWidth = m_pdfWriter.measureTextWidth(span->text, span->style);
        currentX += spanWidth;
    }
}
// -----------------------------------------------------------------
// 3. FUNCȚII DE RANDARE PRIMITIVE (Apelează Wrapper-ul)
// -----------------------------------------------------------------

// În RtfToPdfConverter.cpp
void RtfToPdfConverter::processRtfTable(const RtfTable& table) {
    LOG_WARNING(L"Randare tabel RTF: Incepe. Randuri: " + std::to_wstring(table.rows.size()));

    // Obține lățimile coloanelor o singură dată
    if (table.rows.empty()) {
        LOG_WARNING(L"Tabelul este gol, randare anulată.");
        return;
    }

    // ⭐ Calculăm înălțimea minimă a întregului tabel pentru paginare inițială
    // (O estimare rapidă: 1 rând * 12pt înălțime)
    double estimatedHeight = table.rows.size() * 12.0;
    newLineAndCheckPageBreak(estimatedHeight);

    // Avansăm cursorul Y la începutul tabelului
    double tableStartTopY = m_currentY;

    // 2. Procesează fiecare rând
    for (size_t i = 0; i < table.rows.size(); ++i) {
        processRtfRow(table.rows[i], table.columnWidthsPt);
    }

    // Nu avansăm m_currentY aici, deoarece processRtfRow și processRtfCell
    // vor gestiona înălțimea.

    // Dacă doriți să desenați borduri de tabel, puteți face apeluri PDF de desenare AICI
    // folosind tableStartTopY și m_currentY.
}

std::wstring paragraphToText(const RtfParagraph& paragraph) {
    std::wstring result;
    for (const auto& span : paragraph.spans) {
        result += span.text;   // concatenăm textul fiecărui span
    }
    return result;
}



std::wstring extractCellText(const RtfCell& cell) {
    std::wstring result;
    for (const auto& block : cell.content) {
        if (const RtfParagraph* paragraph = dynamic_cast<const RtfParagraph*>(block.get())) {
            // presupunem că RtfParagraph are o metodă getText()
            result += paragraphToText(*paragraph);

           // result += L"\n"; // separăm paragrafele
        }
        // aici poți trata și alte tipuri de block (liste, imagini etc.)
    }
    return result;
}




void RtfToPdfConverter::processRtfRow(const RtfRow& row, const std::vector<double>& colWidths) {

    if (row.cells.empty() || colWidths.empty()) {
        LOG_WARNING(L"Rand rând tabel: celule sau lățimi de coloană lipsă.");
        return;
    }

    // Variabilă care urmărește înălțimea maximă reală de care are nevoie rândul.
    double maxRowHeight = 0.0;

    // Lista înălțimilor reale folosite de celule, pentru a avansa m_currentY corect.
    std::vector<double> cellHeightsUsed;

    // ⭐ Marginea stângă a documentului (offset-ul tabelului pe pagină)
    double tableMarginLeftPt = m_marginLeft;

    // Variabila care urmărește limita stângă ABSOLUTĂ a celulei curente pe pagină.
    double cellStartBoundaryPt = tableMarginLeftPt;

    // Toate celulele din rând încep la aceeași înălțime Y
    double cellStartTopY = m_currentY;

    if (row.cells.size() != colWidths.size())
        LOG_ERROR(L"Lipsesc dimensiuni: Numar celule != Numar latimi");

    // ---------------------------------------------------------------------
    // 1. RANDAREA CONȚINUTULUI ȘI MĂSURAREA ÎNĂLȚIMII REALE
    // ---------------------------------------------------------------------

    for (size_t j = 0; j < row.cells.size() && j < colWidths.size(); ++j) {

        // Limita ABSOLUTĂ RTF (de la 0)
        double rtfEndBoundaryPt = colWidths[j];

        // Granița ABSOLUTĂ DE SFÂRȘIT pe pagină
        double cellEndBoundaryPt = tableMarginLeftPt + rtfEndBoundaryPt;

        // Lățimea Celulei
        double cellWidth = cellEndBoundaryPt - cellStartBoundaryPt;

        // Poziția de start a randării celulei
        double cellStartX = cellStartBoundaryPt;

        if (cellWidth <= 0.0) {
            LOG_ERROR(L"Lățime celulă non-pozitivă detectată. Sărire celulă.");
            cellStartBoundaryPt = cellEndBoundaryPt;
            continue;
        }

        // ⭐ Randarea propriu-zisă a conținutului celulei
        // processRtfCell() va reseta m_currentY la cellStartTopY la final.
        double actualHeight = processRtfCell(row.cells[j], cellStartX, cellStartTopY, cellWidth, 0.0);

        cellHeightsUsed.push_back(actualHeight);
        // ⭐ Actualizăm înălțimea maximă reală
        maxRowHeight = std::max<double>(maxRowHeight, actualHeight);

        // Actualizăm limita ABSOLUTĂ de start pentru următoarea celulă.
        cellStartBoundaryPt = cellEndBoundaryPt;
    }

    // ---------------------------------------------------------------------
    // 2. VERIFICARE PAGINARE FINALĂ ȘI AVANSARE (UNICA) PE Y
    // ---------------------------------------------------------------------

    // Verificare Paginare: Asigură-te că rândul încape în pagina curentă
    // (Această verificare ar trebui să folosească înălțimea reală calculată)
    // ⚠️ NOTĂ: Dacă se face schimb de pagină aici, bordurile ar trebui randate
    // pe pagina nouă, dar pentru simplitate, presupunem că rămânem pe aceeași pagină.
    newLineAndCheckPageBreak(maxRowHeight);

    // ---------------------------------------------------------------------
    // 3. DESENAREA BORDURILOR CELULELOR (Folosind înălțimea MAXIMĂ)
    // ---------------------------------------------------------------------

    double currentCellStartX = tableMarginLeftPt; // Resetăm X-ul pentru desenare

    // Coordonatele Y în sistemul PDF (0 la bază)
    double yBottomPdf = m_pageHeight - cellStartTopY - maxRowHeight; // Partea de jos a rândului
    double yTopPdf = m_pageHeight - cellStartTopY;                    // Partea de sus a rândului

    for (size_t j = 0; j < row.cells.size() && j < colWidths.size(); ++j) {
        const auto& cell = row.cells[j];

        double rtfEndBoundaryPt = colWidths[j];
        double cellEndBoundaryPt = tableMarginLeftPt + rtfEndBoundaryPt;
        double cellWidth = cellEndBoundaryPt - currentCellStartX;

        if (cellWidth > 0.0) {
            double xLeft = currentCellStartX;
            double xRight = cellEndBoundaryPt;
         //   LOG_ERROR(L"Celula " + std::to_wstring(j) + L" - Borduri setate:");
         //   LOG_ERROR(L"  Left set: " + std::to_wstring(cell.borders.left.isSet()) + L", Width: " + std::to_wstring(cell.borders.left.widthTwips));
         //   LOG_ERROR(L"  Top set: " + std::to_wstring(cell.borders.top.isSet()) + L", Width: " + std::to_wstring(cell.borders.top.widthTwips));
         //   LOG_ERROR(L"  Right set: " + std::to_wstring(cell.borders.right.isSet()) + L", Width: " + std::to_wstring(cell.borders.right.widthTwips));
         //   LOG_ERROR(L"  Bottom set: " + std::to_wstring(cell.borders.bottom.isSet()) + L", Width: " + std::to_wstring(cell.borders.bottom.widthTwips));
            // ... și pentru Bottom/Right.
            // Bordura Stânga (\clbrdrl)
            renderCellBorder(cell.borders.left, xLeft, yTopPdf, xLeft, yBottomPdf);

            // Bordura Dreapta (\clbrdrr)
            renderCellBorder(cell.borders.right, xRight, yTopPdf, xRight, yBottomPdf);

            // Bordura Sus (\clbrdrt)
            renderCellBorder(cell.borders.top, xLeft, yTopPdf, xRight, yTopPdf);

            // Bordura Jos (\clbrdrb)
            renderCellBorder(cell.borders.bottom, xLeft, yBottomPdf, xRight, yBottomPdf);
        }

        currentCellStartX = cellEndBoundaryPt;
    }

    // ---------------------------------------------------------------------
    // 4. AVANSAREA FINALĂ A CURSORULUI Y
    // ---------------------------------------------------------------------

    // Avansarea cursorului Y O SINGURĂ DATĂ, cu înălțimea maximă reală
    m_currentY += maxRowHeight;

    //LOG_DEBUG(L"Randare rând tabel RTF finalizată. Înălțime rând: " + std::to_wstring(maxRowHeight) + L"pt. Cursor Y avansat.");
}

double RtfToPdfConverter::processRtfCell(const RtfCell& cell, double cellStartX, double cellStartY, double cellWidth, double cellHeight) {
    // Loghează poziția și lățimea REALĂ a celulei.
    //LOG_DEBUG(L"Randare celulă tabel RTF - Incepe la X=" + std::to_wstring(cellStartX) + L", W=" + std::to_wstring(cellWidth));

    // Salvează starea globală a convertorului înainte de a o modifica pentru celulă.
    double original_marginLeft = m_marginLeft;
    double original_contentWidth = m_contentWidth;
    double original_currentY = m_currentY; // Salvează cursorul global

    double totalCellHeightUsed = 0.0;
    // NU salvăm original_currentY, deoarece processRtfParagraph va avansa cursorul Y 
    // în interiorul celulei, iar processRtfRow va decide înălțimea finală a rândului 
    // pe baza maxRowHeight, nu pe baza înălțimii randate a celulei.

    // ⭐ 1. APLICAREA PADDING-ULUI (Conversie din Twips în Puncte PDF)

    // NOTĂ: Dacă folosiți ConvertUtils, presupun că twipsToPoints este disponibil.
    double paddingLeftPt = ConvertUtils::twipsToPoints(cell.padding.leftTwips);
    double paddingRightPt = ConvertUtils::twipsToPoints(cell.padding.rightTwips);
    double paddingTopPt = ConvertUtils::twipsToPoints(cell.padding.topTwips);
    double paddingBottomPt = ConvertUtils::twipsToPoints(cell.padding.bottomTwips);

    // 2. Setează limitele de randare pentru conținutul celulei

    // a) Noul m_marginLeft (Poziția de start a textului)
    // = Granița stânga a celulei + Padding Stânga
    m_marginLeft = cellStartX + paddingLeftPt;

    // b) Noua m_contentWidth (Lățimea reală disponibilă pentru text)
    // = Lățimea totală a celulei - Padding Stânga - Padding Dreapta
    m_contentWidth = cellWidth - paddingLeftPt - paddingRightPt;

    // c) Noul m_currentY (Poziția de start a primei linii de text)
    // = Partea de sus a rândului + Padding Sus
    m_currentY = cellStartY + paddingTopPt;

    // Verificare pentru lățime negativă
    if (m_contentWidth <= 0.0) {
        LOG_ERROR(L"Lățime de conținut negativă/zero din cauza padding-ului excesiv. Sărire celulă.");
        // Restaurare stare
        m_marginLeft = original_marginLeft;
        m_contentWidth = original_contentWidth;
        m_currentY = original_currentY;
        return 0.0;
    }

    // 2. Procesează blocurile de conținut ale celulei
    for (const auto& block : cell.content) {
        if (const RtfParagraph* paragraph = dynamic_cast<const RtfParagraph*>(block.get())) {
            // Randarea paragrafului folosește NOUA m_marginLeft și NOUA m_contentWidth.
            // De asemenea, processRtfParagraph avansează m_currentY.
            double height = processRtfParagraph(*paragraph);
            totalCellHeightUsed += height; // Acumulează înălțimea
        }
        // ... alte tipuri de block (liste etc.)
    }

    // 4. Calculul Înălțimii Totale Folosite (cu Padding)
    // Înălțimea totală de care are nevoie celula (folosită pentru maxRowHeight)
    // = Padding Sus + Înălțimea Randată a Conținutului + Padding Jos
    totalCellHeightUsed += paddingTopPt + paddingBottomPt;

    // 5. Restaurarea stării globale la valorile de dinaintea celulei.
    m_marginLeft = original_marginLeft;
    m_contentWidth = original_contentWidth;
    m_currentY = original_currentY;

    // 6. Returnează înălțimea totală de care a avut nevoie celula (cu padding).
    return totalCellHeightUsed;
}

void RtfToPdfConverter::processRtfSpan(const RtfSpan& span) {
    // Nu ar trebui să fie apelate direct.
}

void RtfToPdfConverter::applyStyleToWriter(const Style& style) {
    // Nu face nimic în această implementare (stateless styling).
}

/*
void RtfToPdfConverter::renderCellBorder(
    const BorderSpec& spec,
    double x1, double y1, double x2, double y2)
{
    // 1. Verifică condițiile de bază (bordură definită și stil Single)
    if (spec.isSet() && spec.style == RtfBorderStyle::Single) {

        // Convertim lățimea din Twips în Puncte (1pt = 20 Twips)
        double widthPt = static_cast<double>(spec.widthTwips) / 20.0;

        // Asigură-te că grosimea este pozitivă
        if (widthPt <= 0.0) {
            return;
        }

        // 2. Definește culoarea (Negru)
        // Presupunând că ColorRgb are membri r, g, b (0.0 la 1.0)
        ColorRgb blackColor = { 0.0, 0.0, 0.0 }; // Negru

        // 3. Apelul la PdfWriterWrapper::addLine
        // Parametrii: x1, y1, x2, y2, thickness, color

        RenderInstruction instruction;
        instruction.x = x1;
        instruction.y = y1;
        instruction.width = x2;
        instruction.height = y2;
        
        //instruction.style = spec.style;
        instruction.renderFunction = L"line";
        m_renderQueue.push_back(instruction);

        //pdfWriter.addLine(x1, y1, x2, y2, widthPt, blackColor);

        // Nu este necesar să ne facem griji pentru `pdfWriter.setStrokeWidth` sau 
        // `pdfWriter.setStrokeColor` deoarece acestea sunt gestionate intern de `addLine`.
    }

    // TODO: Adaugă suport pentru RtfBorderStyle::Double etc.
    // Pentru a adăuga stiluri punctate/duble, ar trebui să implementați
    // o logică complexă de desenare a mai multor linii sau de setare a pattern-ului
    // de linie (`m_currentPageContext->d()`) înainte de `m_currentPageContext->s()`.
}
*/

void RtfToPdfConverter::renderCellBorder(
    const BorderSpec& spec,
    double x1, double y1, double x2, double y2)
{
    if (spec.isSet() && spec.style == RtfBorderStyle::Single) {

        double widthPt = static_cast<double>(spec.widthTwips) / 20.0;
        if (widthPt <= 0.0) return;

        ColorRgb blackColor = { 0.0, 0.0, 0.0 };

        RenderInstruction instruction;

        // 1. Coordonatele Punctului 1 (Start)
        instruction.x = x1; // Coordonata X1
        instruction.y = y1; // Coordonata Y1

        // 2. ⭐ Salvarea Proprietăților Liniei în Style

        // Salvează Coordonatele Punctului 2 (End) în boxModel (unde nu sunt folosite)
        instruction.width = x2;
        instruction.height = y2;

        // Salvează Grosimea și Culoarea liniei
        instruction.style.boxModel.borderLeftWidth = widthPt; // Folosim un câmp de grosime
        instruction.style.borderColor = blackColor;

        instruction.renderFunction = L"line";

        // Asigurați-vă că este setat și numărul paginii curente
        // instruction.page_number = m_currentPageNumber; 

        m_renderQueue.push_back(instruction);
    }
}

void RtfToPdfConverter::renderFooter() {
    // Presupunând că Rtf::getFooterBlocks() returnează this->footerBlocks
    if (m_rtfDocument.getFooterBlocks().empty()) return;

    const double TWIPS_PER_POINT = 20.0;
    double marginBottomPt = m_rtfDocument.getPageInfo().getMarginBottomTwips() / TWIPS_PER_POINT;

    // Y de start (sus în jos)
    double currentFooterY = m_pageHeight - marginBottomPt;
    currentFooterY -= 5.0; // Micul ajust de 5pt în sus

    double originalY = m_currentY;
    m_currentY = currentFooterY;

    // Folosim o înălțime de linie standard (de ex. 8pt, fs16)
    double lineHeight = 8.0 * 1.2;

    // Iterează prin toate blocurile din footer
    for (const auto& block : m_rtfDocument.getFooterBlocks()) {
        double heightConsumed = 0.0;

        if (const RtfParagraph* paragraph = dynamic_cast<const RtfParagraph*>(block.get())) {
            heightConsumed = renderFooterParagraph(*paragraph, lineHeight);

        }
        else if (const RtfTable* table = dynamic_cast<const RtfTable*>(block.get())) {
            // ⭐ Aici intră footer-ul cu numărul paginii și data!
            heightConsumed = renderFooterTable(*table); // Urmează implementarea

        }
        else {
            // Bloc necunoscut
        }

        m_currentY += heightConsumed;
    }

    m_currentY = originalY;
}

// În RtfToPdfConverter.cpp
double RtfToPdfConverter::renderFooterTable(const RtfTable& table) {
    if (table.rows.empty()) return 0.0;

    const RtfRow& row = table.rows[0];
    double currentX = m_marginLeft;
    double yBaseline = m_pageHeight - m_currentY;
    double maxRowHeight = 0.0; // Pentru a ști cât de mult să avansăm Y-ul

    // 1. Randarea celulelor rândului (Rândul este deja în footer)
    for (size_t i = 0; i < row.cells.size(); ++i) {
        const RtfCell& cell = row.cells[i];

        // Calculul lățimii celulei
        double cellWidth = 0.0;
        if (i < table.columnWidthsPt.size()) {
            // Lățimea celulei este diferența dintre marginea dreaptă a celulei N+1 și N
            cellWidth = table.columnWidthsPt[i] - (i > 0 ? table.columnWidthsPt[i - 1] : 0.0);
        }
        else {
            // Aceasta e o eroare de parsare/configurare RTF (ar trebui să existe un cellx final)
            continue;
        }

        double cellStartAbsX = currentX; // Poziția X absolută de unde începe celula

        // 2. Randarea conținutului celulei
        for (const auto& block : cell.content) {
            if (const RtfParagraph* paragraph = dynamic_cast<const RtfParagraph*>(block.get())) {

                // Măsoară textul din paragraf (pe un singur rând)
                double contentWidth = 0.0;
                for (const auto& span : paragraph->spans) {
                    contentWidth += m_pdfWriter.measureTextWidth(span.text, span.style);
                }

                // A. Calculează X de start al textului în interiorul celulei (pentru aliniere: Left, Center, Right)
                double x_start_in_cell = cellStartAbsX + calculateXOffsetForAlignment(
                    paragraph->style.textAlign,
                    cellWidth,
                    contentWidth
                );

                double drawX = x_start_in_cell;

                // B. Desenează span-urile
                for (const auto& span : paragraph->spans) {
                    // 💡 IMPORTANT: Trebuie să substituiți câmpurile RTF (\chpgn, \field) aici
                    std::wstring textToDraw = span.text;

                    // Substituție câmpuri (ex: Pag. N / M)
                    textToDraw = replaceRtfFields(textToDraw, m_currentPageNumber, m_totalPagesCount);

                    RenderInstruction instruction;
                    instruction.x = drawX;
                    instruction.y = yBaseline;
                    instruction.text_content = textToDraw;
                    instruction.style = span.style;
                    instruction.renderFunction = L"text";

                    identifyGlobalVars(instruction.text_content, instruction.globalVars);

                    m_renderQueue.push_back(instruction);

                    //pdfWriter.addTextWithSyle(drawX, yBaseline, textToDraw, span.style);
                    drawX += m_pdfWriter.measureTextWidth(textToDraw, span.style);
                }

                // C. Actualizează înălțimea maximă (pentru rând)
                maxRowHeight = std::max<double>(maxRowHeight, paragraph->style.fontSize * 1.2);
            }
        }

        // 3. Avansăm la poziția X de start a următoarei celule
        currentX = cellStartAbsX + cellWidth;
    }

    // Returnăm înălțimea maximă a rândului
    return maxRowHeight > 0.0 ? maxRowHeight : 12.0;
}

// 💡 Funcția ajutătoare (Placeholder, trebuie implementată de dvs.)
double RtfToPdfConverter::calculateXOffsetForAlignment(const std::wstring& align, double cellWidth, double contentWidth) {
    if (align == L"center") {
        return (cellWidth - contentWidth) / 2.0;
    }
    else if (align == L"right") {
        return cellWidth - contentWidth;
    }
    // Default: left
    return 0.0;
}

// 💡 Funcția ajutătoare (Placeholder, trebuie implementată de dvs.)
std::wstring RtfToPdfConverter::replaceRtfFields(const std::wstring& text, int currentPage, int totalPages) {
    std::wstring result = text;
    // ... Logica de înlocuire (\chpgn -> currentPage, \field{\*\fldinst NUMPAGES} -> totalPages)
    // De exemplu:
    std::size_t pos_chpgn = result.find(L"\\chpgn");
    if (pos_chpgn != std::wstring::npos) {
        //LOG_ERROR(L"PADGIIIINI");
        result.replace(pos_chpgn, 6, std::to_wstring(currentPage));
    }
    // Logica pentru NUMPAGES e mai complexă
    // Simplificare:
  
//    std::size_t pos_num_pages = result.find(L"NUMPAGES");
//    if (pos_num_pages != std::wstring::npos) {
//        result.replace(pos_num_pages, 8, std::to_wstring(totalPages));
//    }
  
    return result;
}

// RtfToPdfConverter::renderFooterParagraph (Versiune simplificată și corectată)
double RtfToPdfConverter::renderFooterParagraph(const RtfParagraph& paragraph, double lineHeight) {
    double heightConsumed = lineHeight;

    // 1. Măsoară lățimea totală a textului din paragraf (aplicând SUBSTITUȚIA)
    double totalTextWidth = 0.0;
    for (const auto& span : paragraph.spans) {
        std::wstring textToDraw = span.text;
        // Aplică substituția pentru o măsurare precisă
        textToDraw = replaceRtfFields(textToDraw, m_currentPageNumber, m_totalPagesCount);
        totalTextWidth += m_pdfWriter.measureTextWidth(textToDraw, span.style);
    }

    // 2. Calculează X de start (bazat pe totalTextWidth substituit)
    double x_start_for_rendering = calculateLineXStart(
        paragraph.style.textAlign,
        m_contentWidth,
        totalTextWidth,
        m_marginLeft
    );

    double currentX = x_start_for_rendering;
    double yBaseline = m_pageHeight - m_currentY;

    // 3. Iterează și desenează (aplicând din nou SUBSTITUȚIA)
    for (const auto& span : paragraph.spans) {
        const Style& spanStyle = span.style;

        // ⭐ Pasul cheie: Substituție pentru randare
        
        
        std::wstring textToDrawFinal = replaceRtfFields(
            span.text,
            m_currentPageNumber,
            m_totalPagesCount
        );
        
        //std::wstring textToDrawFinal = span.text;
        if (textToDrawFinal == L"\\t") {

            // 1. Identifică următoarea oprire de tabulator (din paragraph.style)
            // Logica complexă presupune:
            //    a) Găsirea celei mai apropiate opriri (în puncte) după currentX.
            //    b) Aplicarea tipului de aliniere (stânga, centru, dreapta)
            /*
            double nextTabStopX = findNextTabStopPosition(
                paragraph.style.tabStops, // Opririle definite în RTF, convertite în PT
                currentX
            );

            // 2. Setează noua poziție X. 
            // Dacă nu găsiți o oprire de tabulator, folosiți un avans standard (ex: 36pt).
            if (nextTabStopX > currentX) {
                currentX = nextTabStopX;
            }
            else {
                // Fără oprire explicită, folosește un salt implicit (ex: 0.5 inchi = 36pt)
                currentX += 36.0;
            }
            */
            currentX += TAB_WIDTH;
            // Săriți peste randarea caracterului \t
            continue;
        }

        // Randarea textului span-ului
        
        RenderInstruction instruction;
        instruction.x = currentX;
        instruction.y = yBaseline;
        instruction.text_content = textToDrawFinal;
        instruction.style = spanStyle;
        instruction.renderFunction = L"text";

        identifyGlobalVars(instruction.text_content, instruction.globalVars);

        m_renderQueue.push_back(instruction);

        //pdfWriter.addTextWithSyle(currentX, yBaseline, textToDrawFinal, spanStyle);

        // Avansăm cursorul X folosind lățimea textului SUBSTITUIT!
        double spanWidth = m_pdfWriter.measureTextWidth(textToDrawFinal, spanStyle);
        currentX += spanWidth;
    }

    return heightConsumed;
}


bool RtfToPdfConverter::finalizeAndPaint() {

    int current_render_page = 0;
    /*
    if (!m_pdfWriter.initialize(outputFilePath, pageWidth, pageHeight)) {
        LOG_ERROR(L"RtfToPdfConverter::finalizeAndPaint: Eroare la inițializarea PdfWriter.");
        return false;
    }
    */
   


    for (const auto& instruction : m_renderQueue) {
        if (instruction.renderFunction == L"text") {
            LOG_ERROR(L"DESENEZ textul: \"" + instruction.text_content + L"\" cu coordonatele:X=" +
                std::to_wstring(instruction.x) + L" Y=" + std::to_wstring(instruction.y));
             //   L" width=" + std::to_wstring(instruction.width) + L" height=" + std::to_wstring(instruction.height) +
             //   L" Final X=" + std::to_wstring(instruction.x + instruction.width) + L" Final Y=" + std::to_wstring(instruction.y + instruction.height));
            //LOG_INFO(L"Page start:" + to_wstring<int>(instruction.element.layout_start_page_index) + L" Page end : "+ to_wstring<int>(instruction.element.layout_end_page_index));
            //LOG_INFO(L"RuleName:" + instruction.style.ruleName);
            //LOG_INFO(L"FontFamily:" + instruction.style.fontFamily);
            //LOG_INFO(L"FontWeight:" + instruction.style.fontWeight);
            std::wstring final_text_to_draw = instruction.text_content;
            int page_index_for_substitution = current_render_page;
            // ----------------------------------------------------
    // ⭐ Substituția bazată pe vectorul de variabile
    // ----------------------------------------------------
            for (const auto& varName : instruction.globalVars) {
                if (m_globalVarResolvers.count(varName)) {

                    // 1. Obține valoarea finală (string) de la resolver
                    std::wstring resolvedValue = m_globalVarResolvers.at(varName)(page_index_for_substitution);

                    // 2. Substituie variabila în textul brut
                    size_t pos = final_text_to_draw.find(varName);
                    if (pos != std::wstring::npos) {
                        final_text_to_draw.replace(pos, varName.length(), resolvedValue);
                    }
                }
            }

            m_pdfWriter.addTextWithSyle(instruction.x,
                instruction.y,
                final_text_to_draw, instruction.style);
        }
        else  if (instruction.renderFunction == L"startPage") {
            m_pdfWriter.startPage(instruction.width, instruction.height);
            current_render_page++;
        }
        else if (instruction.renderFunction == L"endPage") {
            m_pdfWriter.endPage();
        }
        else if (instruction.renderFunction == L"line") {
            // Citește x2 și y2 din proprietățile boxModel
            double x2 = instruction.width;
            double y2 = instruction.height;

            // Citește grosimea din proprietățile boxModel
            double thickness = instruction.style.boxModel.borderLeftWidth;

            // Citește culoarea din style
            ColorRgb color = instruction.style.borderColor;

            m_pdfWriter.addLine(
                instruction.x,
                instruction.y,
                x2,
                y2,
                thickness,
                color
            );
          //  m_pdfWriter.addLine(instruction.x, instruction.y, instruction.width, instruction.height, widthPt, blackColor);
        }
    }

    if (!m_pdfWriter.finalize()) {
        LOG_ERROR(L"RtfToPdfConverter::finalizeAndPaint: Eroare la finalizarea documentului PDF.");
        return false;
    }
    return true;
}


void RtfToPdfConverter::initializeGlobalVarResolvers() {
    // Folosim o funcție lambda care captează m_totalPagesCount final

    // 1. NUMPAGES: Variabilă globală, valoare cunoscută la final.
    m_globalVarResolvers[L"NUMPAGES"] = [this](int page_index) -> std::wstring {
        // m_totalPagesCount este setat la finalul Pass-ului 1.
        return std::to_wstring(this->m_totalPagesCount);
    };

    // 2. \chpgn (Page Number): Variabilă care depinde de context (pagina curentă).
    m_globalVarResolvers[L"\\chpgn"] = [](int page_index) -> std::wstring {
        // Folosim indexul paginii primit ca parametru (din RenderInstruction::page_number)
        return std::to_wstring(page_index);
    };

    // (Aici ați adăuga și alte câmpuri: L"DATE", L"TIME", etc.)
}


void RtfToPdfConverter::identifyGlobalVars(const std::wstring& text, std::vector<std::wstring>& vars) {
    // Note: Folosim find() pentru a verifica prezența câmpului RTF

    // 1. NUMPAGES
    if (text.find(L"NUMPAGES") != std::wstring::npos) {
        vars.push_back(L"NUMPAGES");
    }

    // 2. \chpgn (Page Number)
 //   if (text.find(L"\\chpgn") != std::wstring::npos) {
//        vars.push_back(L"\\chpgn");
//    }

    // 3. (Adăugați și alte câmpuri globale dacă e necesar: DATE, TIME, etc.)
}
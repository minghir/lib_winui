#include "rtf.hpp"
#include "../ui/ConsoleManager.hpp"
#include "../stringUtils.hpp"
#include <filesystem>
bool Rtf::load(const std::wstring& filePath) {
    // 1. Curăță blocurile existente (dacă se apelează de mai multe ori)
    blocks.clear();

    // 2. Citirea fișierului RTF
    //std::wifstream file(filePath, std::ios::binary);
    //std::ifstream file(filePath, std::ios::binary);
    std::ifstream file{ std::filesystem::path(filePath), std::ios::binary };
    if (!file.is_open()) {
        std::wcerr << L"Eroare: Nu s-a putut deschide fișierul RTF: " << filePath << std::endl;
        return false;
    }

    // Setarea localei (important pentru fișierele text cu caractere non-ASCII)
    // Deși RTF-ul folosește caractere ASCII pentru comenzi, conținutul poate fi în UTF-8/Wide Char.
    // Presupunând că citiți fișierele ca wide string (std::wstring)
    //file.imbue(std::locale(""));

    // Citirea întregului conținut în std::wstring
    //std::wstringstream buffer;
    std::stringstream buffer;
    buffer << file.rdbuf();
    //std::wstring rtfContent = buffer.str();
    std::string rtfContent = buffer.str();

    if (rtfContent.empty()) {
        std::wcerr << L"Avertisment: Fișierul RTF este gol sau nu a putut fi citit." << std::endl;
        return true; // Considerăm că un fișier gol nu este o eroare fatală
    }

    try {
     
        blocks = parseRtfContent(rtfContent); // Presupunând că această funcție există

        std::wcout << L"Informație: Parsarea RTF a avut succes. S-au găsit "
            << blocks.size() << L" blocuri." << std::endl;

    }
    catch (const std::exception& e) {
        // Conversia mesajului de eroare din char* în std::wstring
        std::string errMsg = e.what();
        std::wstring wErrMsg(errMsg.begin(), errMsg.end());
        std::wcerr << L"Eroare la parsarea conținutului RTF: " << wErrMsg << std::endl;
        blocks.clear(); // Asigură-te că nu păstrezi blocuri parțial parsate
        return false;
    }

    return !blocks.empty();
}


// Funcție utilitară pentru a crea indentarea bazată pe adâncime
std::wstring getIndent(int depth) {
    std::wstring indent = L"";
    for (int i = 0; i < depth; ++i) {
        indent += L"  |"; // Folosim | pentru a marca adâncimea
    }
    return indent + L"-- ";
}

// Funcție recursivă principală pentru orice Block
void printRtfBlock(const RtfBlock& block, int depth) {
    // Încearcă să convertească (downcast) la tipurile derivate

    // Tentativă de a printa ca RtfParagraph
    if (const auto* paragraph = dynamic_cast<const RtfParagraph*>(&block)) {
        printRtfParagraph(*paragraph, depth);
        return;
    }

    // Tentativă de a printa ca RtfTable
    if (const auto* table = dynamic_cast<const RtfTable*>(&block)) {
        printRtfTable(*table, depth);
        return;
    }

    // În cazul în care se adaugă noi tipuri de blocuri care nu sunt gestionate
    LOG_WARNING(getIndent(depth) + L"Tip RtfBlock necunoscut la adresa: " + std::to_wstring((uintptr_t)&block));
}

void printRtfParagraph(const RtfParagraph& paragraph, int depth) {
    const std::wstring indent = getIndent(depth);

    // INFO: Antetul Paragrafului
    LOG_INFO(indent + L"PARAGRAPH [Style: Font " + std::to_wstring(paragraph.style.fontSize) + L"pt]");

    // DEBUG: Conținutul Span-urilor
    for (size_t i = 0; i < paragraph.spans.size(); ++i) {
        const auto& span = paragraph.spans[i];

        std::wstring spanInfo = L"[" + std::to_wstring(i) + L"] ";
        spanInfo += span.text.length() > 30
            ? span.text.substr(0, 27) + L"..."
            : span.text;

        LOG_DEBUG(getIndent(depth + 1) + L"SPAN (Chars: " + std::to_wstring(span.text.length()) + L"): '" + spanInfo + L"'");
    }
}

std::wstring getBorderStyleName(RtfBorderStyle style) {
    switch (style) {
    case RtfBorderStyle::None: return L"None";
    case RtfBorderStyle::Single: return L"Single";
    case RtfBorderStyle::Double: return L"Double";
    default: return L"Unknown";
    }
}

void printBorderSpec(const std::wstring& name, const BorderSpec& spec, int depth) {
    const std::wstring indent = getIndent(depth);

    // Verificăm dacă bordura este setată (stil diferit de None SAU lățimea > 0)
    if (spec.style != RtfBorderStyle::None || spec.widthTwips > 0) {
        LOG_DEBUG(indent + L"-> " + name + L": Style: " + getBorderStyleName(spec.style) +
            L", Width: " + std::to_wstring(spec.widthTwips) + L" twips");
    }
    else {
        LOG_DEBUG(indent + L"-> " + name + L": Not Set (None)");
    }
}

void printRtfCell(const RtfCell& cell, int depth) {
    const std::wstring indent = getIndent(depth);

    // SUCCESS: Antetul Celulei
    LOG_SUCCESS(indent + L"CELL (Colspan: " + std::to_wstring(cell.colspan) +
        L", Rowspan: " + std::to_wstring(cell.rowspan) + L")");

    // --- SECȚIUNE NOUĂ: Afișare Padding ---
    LOG_INFO(indent + L"  [Padding - Twips]:");
    int paddingDepth = depth + 1;
    const std::wstring paddingIndent = getIndent(paddingDepth);

    LOG_DEBUG(paddingIndent + L"-> Top: " + std::to_wstring(cell.padding.topTwips) + L" twips");
    LOG_DEBUG(paddingIndent + L"-> Bottom: " + std::to_wstring(cell.padding.bottomTwips) + L" twips");
    LOG_DEBUG(paddingIndent + L"-> Left: " + std::to_wstring(cell.padding.leftTwips) + L" twips");
    LOG_DEBUG(paddingIndent + L"-> Right: " + std::to_wstring(cell.padding.rightTwips) + L" twips");

    // --- Afișare Borduri (Secțiune existentă) ---
    LOG_INFO(indent + L"  [Borders]:");
    int borderDepth = depth + 1; // Adâncime pentru detalii borduri

    // Afișează fiecare specificație de bordură
    printBorderSpec(L"Left", cell.borders.left, borderDepth);
    printBorderSpec(L"Top", cell.borders.top, borderDepth);
    printBorderSpec(L"Right", cell.borders.right, borderDepth);
    printBorderSpec(L"Bottom", cell.borders.bottom, borderDepth);

    // DEBUG: Traversăm recursiv conținutul celulei (care sunt RtfBlock-uri)
    LOG_INFO(indent + L"  [Content Blocks]:");
    for (const auto& block : cell.content) {
        if (block) {
            printRtfBlock(*block, depth + 1); // Apelează printRtfBlock cu adâncime mărită
        }
        else {
            LOG_ERROR(getIndent(depth + 1) + L"Eroare: RtfBlock NULL în celulă.");
        }
    }
}

void printRtfTable(const RtfTable& table, int depth) {
    const std::wstring indent = getIndent(depth);

    // WARNING: Antetul Tabelului
    LOG_WARNING(indent + L"TABLE (Rows: " + std::to_wstring(table.rows.size()) + L")");

    // Iterează prin Rânduri
    for (size_t r = 0; r < table.rows.size(); ++r) {
        const auto& row = table.rows[r];
        LOG_WARNING(getIndent(depth + 1) + L"ROW [" + std::to_wstring(r) + L"] (Cells: " + std::to_wstring(row.cells.size()) + L")");

        // Iterează prin Celule
        for (size_t c = 0; c < row.cells.size(); ++c) {
            const auto& cell = row.cells[c];
            printRtfCell(cell, depth + 2); // Apelează printRtfCell
        }
    }
}

// ----------------------------------------------------
// B. Metoda Principală Rtf::print()
// ----------------------------------------------------

void Rtf::print() const {
    LOG(L"\n=======================================================");
    LOG(L"| DUMP STRUCTURĂ DOCUMENT RTF ÎNCĂRCAT (Rtf::print()) |");
    LOG(L"=======================================================");

    if (blocks.empty()) {
        LOG_INFO(L"Documentul RTF nu conține blocuri (gol sau parsare eșuată).");
        return;
    }

    // Traversează colecția de blocuri de nivel superior
    for (size_t i = 0; i < blocks.size(); ++i) {
        const auto& block = blocks[i];

        // Asigură-te că pointerul nu este NULL
        if (block) {
            LOG_INFO(L"Bloc de nivel 0 [" + std::to_wstring(i) + L"]:");
            printRtfBlock(*block, 1); // Începe cu adâncimea 1
        }
        else {
            LOG_FATAL(L"Eroare FATALĂ: Bloc de nivel 0 [" + std::to_wstring(i) + L"] este NULL!");
        }
    }

    LOG(L"=======================================================");
    LOG_SUCCESS(L"Dump-ul structurii a fost finalizat cu succes.");
}


std::vector<std::unique_ptr<RtfBlock>> Rtf::parseRtfContent(const std::string& content) {
    std::vector<std::unique_ptr<RtfBlock>> parsedBlocks;

    if (pageConfigurations.empty()) {
        RtfPage defaultPage;
        // Asigură-te că defaultPage are setări RTF implicite,
        // inclusiv lanscaping dacă e necesar (nu e vizibil aici).
        pageConfigurations.push_back(defaultPage);
    }

    RtfPage& activePageConfig = pageConfigurations.back();
    RtfParseState state(activePageConfig);

    // Setează codificarea implicită (de obicei 1252 pentru ANSI sau 0/Default)
    // Dacă documentul nu specifică \ansicpg, se folosește codificarea sistemului.
    // Lăsăm 0 și aplicăm 1252/1250 doar la conversie.
    // state.ansicpg este setat de \ansicpg.

    //state.currentStyle.fontSize = 24.0; // RTF implicit (12pt)
    state.currentStyle.fontSize = 12.0; // RTF implicit (12pt)
    state.currentParagraph = std::make_unique<RtfParagraph>();

    // Logica principală de traversare
    for (size_t i = 0; i < content.length(); ++i) {
        char c = content[i];
        //unsigned int codePage = state.ansicpg == 0 ? 1252 : state.ansicpg;
        unsigned int codePage = 65001;

        // 1. GESTIUNEA GRUPURILOR STRUCTURALE ({ și })
        if (c == '{') {
            // Salvează contextul (stilul curent) pe stivă
            state.styleStack.push_back(state.currentStyle);
            state.metadataStack.push_back(state.isParsingMetadata);
        }

        else if (c == '}') {
            if (state.parsingFontTable) {
                // Textul din buffer este numele fontului.
                // Îl asociem cu indexul fontului stocat anterior de \fN.
                if (!state.currentTextBuffer.empty() && state.currentFontIndexForTable != -1) {
                    std::wstring fontName = state.currentTextBuffer;

                    // Îndepărtează punct și virgulă final, dacă există
                    if (!fontName.empty() && fontName.back() == L';') {
                        fontName.pop_back();
                    }

                    state.fontTable[state.currentFontIndexForTable] = fontName;
                    LOG_DEBUG(L"FONT TABLE: Font Index " + std::to_wstring(state.currentFontIndexForTable) + L" populat cu: " + fontName);
                }

                // Resetează starea de parsare a fontului la ieșirea din grupul \fonttbl
                state.parsingFontTable = false;
                state.currentFontIndexForTable = -1;
                state.currentTextBuffer.clear(); // Golește buffer-ul care conținea numele fontului
            }

            // Ieșire din grup: finalizează orice paragraf sau tabel neterminat
            auto block = finalizeCurrentParagraph(state);
            if (block) {
                parsedBlocks.push_back(std::move(block));
            }

            if (state.currentTable && state.currentTable) {
                LOG_DEBUG(L"Finalizează tabelul. Adaugă la blocurile parate.");
                parsedBlocks.push_back(std::move(state.currentTable));

                state.inTable = false;
                state.currentCell = nullptr;
            }

            // Restaurează stilul și starea de metadate anterioară
            if (!state.metadataStack.empty()) {
                state.isParsingMetadata = state.metadataStack.back();
                state.metadataStack.pop_back();
            }

            if (!state.styleStack.empty()) {
                state.currentStyle = state.styleStack.back();
                state.styleStack.pop_back();
            }
            else {
                LOG_ERROR(L"Eroare de parsare: '}' fără '{' corespunzător.");
            }
        }

        // 2. GESTIUNEA CUVINTELOR DE CONTROL (\word)
        else if (c == '\\') {
            i++; // Sări peste backslash
            if (i >= content.length()) break;

            char next_char = content[i];

            // 2.1 Tratarea caracterelor speciale (e.g., \{, \}, \\, \tab, \par, \chansicpg)
            if (next_char == '{' || next_char == '}' || next_char == '\\' || next_char == '~' || next_char == '-') {
                // Converteste caracterul special la Wide Char și îl adaugă la buffer
                std::string singleCharStr(&next_char, 1);
                std::wstring wideChar = convertSingleByteToWideChar(singleCharStr, codePage);
                state.currentTextBuffer += wideChar;
                // Nu este un cuvânt de control, nu avansăm i la sfârșitul buclei
                continue;
            }

            // 2.2 Extracția Cuvântului de Control și Parametrului

            // Cazul special pentru comenzile cu un singur caracter, ex: \par, \cell, \row
            // Acestea nu au neapărat litere consecutive sau parametru.
            if (isalpha(next_char)) { // Verifică dacă urmează un cuvânt

                std::string controlWordStr;
                size_t start_i = i;

                // Extrage cuvântul de control (folosind isalpha pentru char)
                while (i < content.length() && isalpha(content[i])) {
                    controlWordStr += content[i];
                    i++;
                }

                // Extrage parametrul numeric (folosind isdigit pentru char)
                int parameter = 0;
                if (i < content.length() && (isdigit(content[i]) || content[i] == '-')) {
                    int sign = 1;
                    if (content[i] == '-') { sign = -1; i++; }
                    while (i < content.length() && isdigit(content[i])) {
                        parameter = parameter * 10 + (content[i] - '0');
                        i++;
                    }
                    parameter *= sign;
                }

                // Tratarea Separatorului (spațiul)
                if (i < content.length() && content[i] == ' ') {
                    i++; // Sări peste spațiul care urmează cuvântului de control
                }

                // Convertim controlWordStr la wstring pentru apelarea handleControlWord
                std::wstring controlWord(controlWordStr.begin(), controlWordStr.end());

                // Aplicarea Cuvântului de Control
                if (controlWord == L"fonttbl" || controlWord == L"colortbl" || controlWord == L"stylesheet") {
                    state.isParsingMetadata = true;
                    LOG_DEBUG(L"Rtf::parseRtfContent: ATIVAT flag-ul de metadate pentru: " + controlWord);
                }
                handleControlWord(state, controlWord, parameter, parsedBlocks);

                // Decrementează i pentru a compensa avansarea la sfârșitul while-urilor
                i--;
            }
            // Dacă nu e un cuvânt, e un caracter de control cu un singur octet (e.g., \')
            else {
                // Aici ar trebui tratate comenzile scurte (e.g., \', \*, \_, etc.)
                // Dacă nu sunt tratate, bucla 'for' va avansa incorect.
                // Pentru simplitate, presupunem că sunt tratate în handleControlWord 
                // sau ca text, dar pentru '`', ar trebui să ne oprim aici și să continuăm.
            }

        }

        // 3. GESTIUNEA TEXTULUI NORMAL (inclusiv diacritice, spații, punctuație)
        // Orice caracter care nu este de control structural sau de cuvânt de control este considerat text.
        else {

            if (state.parsingFontTable) {
                // Colectează caracterul în textul fontului (într-un buffer temporar, sau direct)
                // Deoarece parsați caracter cu caracter, colectăm textul în buffer,
                // apoi îl procesăm la ieșirea din grup sau când întâlnim următorul \fN
                state.currentTextBuffer += c;
                continue; // Trecem la următorul caracter din buclă
            }
            
            if (state.isParsingMetadata) {
                // Dacă suntem în metadate (ex: fonttbl), ignorăm textul ("Arial")
                continue;
            }
            
            // Inițializează paragraful dacă e necesar
            if (state.currentParagraph == nullptr) {
                state.currentParagraph = std::make_unique<RtfParagraph>();
            }

            // Gestionarea spațiilor și a liniilor noi
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                // Ignoră spațiile sau liniile noi redundante la început
                if (state.currentTextBuffer.empty() && state.currentParagraph && state.currentParagraph->spans.empty()) {
                    continue;
                }

                // Înlocuiește \n, \r, \t cu un singur spațiu, deoarece \par gestionează liniile noi.
                if (c == '\n' || c == '\r' || c == '\t') {
                    c = ' '; // Tratează ca un spațiu normal
                }
            }

            // CONVERSIA CRITICĂ: Octet (char CP 1250) -> Wide Char (Unicode)
            // codePage a fost setat la începutul buclei: state.ansicpg == 0 ? 1252 : state.ansicpg;
            std::string singleCharStr(&c, 1);
            //unsigned int codePage = 65001; // Setează codePage (dacă nu este setat corect global)
            std::wstring wideChar = convertSingleByteToWideChar(singleCharStr, codePage);
            
            state.currentTextBuffer += wideChar;
           

           

        }

    } // Sfârșitul buclei for

    // Finalizează ultimul paragraf (dacă există)
    auto block = finalizeCurrentParagraph(state);
    if (block) {
        parsedBlocks.push_back(std::move(block));
    }

    return parsedBlocks;
}

std::unique_ptr<RtfBlock> Rtf::finalizeCurrentParagraph(RtfParseState& state) {
    if (state.isParsingMetadata) {
        state.currentTextBuffer.clear();
        state.currentParagraph = nullptr;
        return nullptr;
    }

    // 1. Finalizează Span-ul curent (adăugând currentTextBuffer la Span)
    finalizeCurrentSpan(state); // Presupunem că finalizeCurrentSpan folosește currentTextBuffer

    // 2. Extrage paragraful finalizat și verifică dacă are conținut
    if (state.currentParagraph == nullptr || state.currentParagraph->spans.empty()) {
        state.currentParagraph = nullptr;
        return nullptr;
    }

    // 3. Verificare dacă paragraful conține doar whitespace
    bool is_empty = true;
    for (const auto& span : state.currentParagraph->spans) {
        if (span.text.find_first_not_of(L" \n\r\t") != std::wstring::npos) {
            is_empty = false;
            break;
        }
    }

    if (is_empty) {
        state.currentParagraph = nullptr;
        return nullptr;
    }

    // Paragraful preia stilul curent
    state.currentParagraph->style = state.currentStyle;

    // 4. LOGICA CRITICĂ PENTRU TABEL
    if (state.inTable && state.currentCell) {
        // Suntem într-un tabel: mutăm paragraful în celula curentă.

        // Mută ownership-ul Paragrafului în celulă
        state.currentCell->content.push_back(std::move(state.currentParagraph));

        // Recreează un Paragraf nou pentru a fi folosit în continuare
        state.currentParagraph = std::make_unique<RtfParagraph>();

        return nullptr; // Nu returnăm un bloc pentru lista globală
    }

    // 5. Nu suntem în tabel: Returnează Paragraful ca bloc global
    else {
        // Mută ownership-ul Paragrafului din state și resetează-l
        std::unique_ptr<RtfBlock> block = std::move(state.currentParagraph);
        //state.currentParagraph = std::make_unique<RtfParagraph>();
        RtfParagraph* paragraph = dynamic_cast<RtfParagraph*>(block.get());

        if (paragraph) {

            // ⭐ LOGICĂ NOUĂ PENTRU FOOTER ȘI HEADER
            if (state.isParsingFooter) {
                // Mută conținutul la containerul footer al clasei Rtf
                
                this->footerBlocks.push_back(std::move(block));
                // Resetează pointerul de stare (deoarece l-am mutat)
                state.currentParagraph = std::make_unique<RtfParagraph>();
                return nullptr; // Nu returnăm un bloc principal
            }
            else if (state.isParsingHeader) {
                // Mută conținutul la containerul header al clasei Rtf
                this->headerBlocks.push_back(std::move(block));
                // Resetează pointerul de stare
                state.currentParagraph = std::make_unique<RtfParagraph>();
                return nullptr; // Nu returnăm un bloc principal
            }
        }

        // Logica existentă (Bloc Principal)
        // Resetează pointerul de stare și returnează blocul principal
        state.currentParagraph = std::make_unique<RtfParagraph>();

        return block;
    }
}


void Rtf::handleControlWord(RtfParseState& state, const std::wstring& word, int param,
    std::vector<std::unique_ptr<RtfBlock>>& parsedBlocks) {
   

    // --- 1. COMANDĂ DE STIL / FORMAT TEXT ---
    if (word == L"fs") {
        finalizeCurrentSpan(state);
        if (!state.currentParagraph) {
            LOG_ERROR(L"CRASH PREVENIT: Paragraful curent este NULL după finalizeCurrentSpan. Re-creare.");
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }
        state.currentStyle.fontSize = param / 2.0;
    }
    
   
    // 2. GESTIONAREA POZIȚIEI BORDURII (\clbrdrl, \clbrdrt, etc.)
    else if (word.rfind(L"clbrdr", 0) == 0) {
        LOG_DEBUG(L"INCERC: " + word + L" LA CELULA:" + std::to_wstring(state.currentRow.cells.size()));

        // Setăm flag-ul Pending (indiferent dacă specificația e setată sau nu)
        if (word == L"clbrdrl") {
            state.borderLeftPending = true;
        }
        else if (word == L"clbrdrt") {
            state.borderTopPending = true;
        }
        else if (word == L"clbrdrb") {
            state.borderBottomPending = true;
        }
        else if (word == L"clbrdrr") {
            state.borderRightPending = true;
        }
        // NOTĂ: Log-ul de APLIC: va fi mutat în applyPendingBorders
    }

    // BORDURI TABELE (Acestea doar setează specificația, fără apply)
   // Setează lățimea bordurii (twips)
    else if (word == L"brdrw") {
        state.currentBorderSpec.widthTwips = param;
        applyPendingBorders(state); // <--- APLICARE CRITICĂ!
    }
    // Setează stilul bordurii (Single, Double, etc.)
    else if (word == L"brdrs") {
        state.currentBorderSpec.style = RtfBorderStyle::Single;
        applyPendingBorders(state); // <--- APLICARE CRITICĂ!
    }
    else if (word == L"brdrdb") { // Exemplu pentru Double
        state.currentBorderSpec.style = RtfBorderStyle::Double;
        applyPendingBorders(state); // <--- APLICARE CRITICĂ!
    }
    // PADDING
    // Setați padding-ul doar dacă există un parametru (N > 0)
    if (word == L"clpadb") {
        if (param > 0) {
            state.currentCellPadding.bottomTwips = param;
            LOG_DEBUG(L"Setat Cell Padding BOTTOM la: " + std::to_wstring(param) + L" twips");
        }
    }
    else if (word == L"clpadt") {
        if (param > 0) {
            state.currentCellPadding.topTwips = param;
            LOG_DEBUG(L"Setat Cell Padding TOP la: " + std::to_wstring(param) + L" twips");
        }
    }
    else if (word == L"clpadl") {
        if (param > 0) {
            state.currentCellPadding.leftTwips = param;
            LOG_DEBUG(L"Setat Cell Padding LEFT la: " + std::to_wstring(param) + L" twips");
        }
    }
    else if (word == L"clpadr") {
        if (param > 0) {
            state.currentCellPadding.rightTwips = param;
            LOG_DEBUG(L"Setat Cell Padding RIGHT la: " + std::to_wstring(param) + L" twips");
        }
    }



    else if (word == L"fonttbl") {
        // Setează AMBELE flag-uri
        state.isParsingMetadata = true;
        state.parsingFontTable = true; // <-- NOU
        LOG_DEBUG(L"Rtf::parseRtfContent: ACTIVAT flag-ul de metadate pentru: fonttbl");
    }
    else if (word == L"b") {

        // 1. Finalizează Span-ul precedent
        finalizeCurrentSpan(state);

        if (!state.currentParagraph) {
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }
        
        if (param == 1) {
            // Acoperă \b1
            state.currentStyle.fontWeight = L"bold";
        }
        else if (param == 0) {
   
            if (param == 0 && state.currentStyle.fontWeight == L"bold") {
                // Aceasta acoperă \b0 (dezactivarea explicită a bold-ului)
                state.currentStyle.fontWeight = L"normal";
            }
            else {
                // Aceasta acoperă \b, \b1, sau orice altceva ce nu e \b0 (dezactivare)
                state.currentStyle.fontWeight = L"bold";
            }
         }
        // APLICĂ STILUL (rămâne același)
        if (state.currentParagraph) {
            state.currentParagraph->style.fontWeight = state.currentStyle.fontWeight;
        }
        /*
        // Logare
        LOG_DEBUG(L"DEBUG BOLD: state.currentStyle.fontWeight SETAT LA: " + state.currentStyle.fontWeight + L" (param: " + std::to_wstring(param) + L")");
        if (state.currentParagraph) {
            LOG_DEBUG(L"Aplicat Bold la Paragraf. Stil curent: " + state.currentParagraph->style.fontWeight);
        }
        */
    }
    else if (word == L"i") {
        finalizeCurrentSpan(state);
        if (!state.currentParagraph) {
            LOG_ERROR(L"CRASH PREVENIT: Paragraful curent este NULL după finalizeCurrentSpan. Re-creare.");
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }
        state.currentStyle.fontStyle = (param != 0) ? L"italic" : L"normal";
    }
    else if (word == L"ul") {
        finalizeCurrentSpan(state);
        if (!state.currentParagraph) {
            LOG_ERROR(L"CRASH PREVENIT: Paragraful curent este NULL după finalizeCurrentSpan. Re-creare.");
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }
        state.currentStyle.textDecoration = (param != 0) ? L"underline" : L"none";
    }

    // --- 2. COMANDĂ DE PARAGRAF / STRUCTURĂ ---
    else if (word == L"par" || word == L"pard") {
        // Finalizează paragraful curent
       // 1. Finalizează paragraful curent (va finaliza și Span-ul din el)
            auto block = finalizeCurrentParagraph(state);
            if (block) {
                parsedBlocks.push_back(std::move(block));
            }

            // 2. Începe un nou paragraf
            state.currentParagraph = std::make_unique<RtfParagraph>();
            //state.currentParagraph->style = state.currentStyle;

            if (word == L"pard") {
                //valor default
                state.currentStyle.textAlign = L"left";
                state.currentStyle.fontWeight = L"normal";
                state.currentStyle.fontStyle = L"normal";
                
            }
            state.currentParagraph->style = state.currentStyle; // Aplică stilul nou
    }
    else if (word == L"line") {
        // Înlocuiește cu un spațiu, un newline, sau un caracter special pentru line-break
        // În scopul parsării HTML/PDF, ar trebui să fie tratat ca o întrerupere de linie
        // de tip hard-break în textul curent.
        finalizeCurrentSpan(state);

        if (!state.currentParagraph) {
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }

      //  if (state.currentParagraph) {
            // Folosim un newline, deși ar trebui să fie un alt tip de element,
            // un newline în buffer e adesea suficient pentru a forța un break în render.
            //state.currentTextBuffer += L"\\n";
        state.currentTextBuffer += L"$line$";
            LOG_ERROR(L"AM GASIT O LINE NOUA");
       // }
            Rtf::finalizeCurrentSpan(state);
    }
    else if (word == L"tab") {
        if (state.isParsingMetadata) return;

        // Finalizează Span-ul anterior (dacă există text)
        finalizeCurrentSpan(state);

        // ⭐ Adaugă un caracter special/marcator în textul buffer
        // Vom folosi un caracter non-tipăribil (ex: L'\u0009' sau un tag)
        state.currentTextBuffer += L"\\t"; // Folosim caracterul standard TAB

        // Finalizează Span-ul cu marcatorul TAB
        finalizeCurrentSpan(state);

        return;
    }
    else if (word == L"page") {
        // \page este un control word care poate apărea oriunde în textul unui paragraf.
        if (state.currentParagraph) {
            // Marcăm întreruperea de pagină cu un caracter sau secvență unică,
            // similar cu \line, dar pentru pagină. Folosim L"\f" (form feed).
            LOG_WARNING(L"ADAUGAREA HARD PAGE BREAK (\\page)");
            state.currentTextBuffer += L"\f"; // Caracterul Form Feed (0x0C)

            // Finalizăm span-ul curent (dacă există text în buffer) pentru a ne asigura
            // că \f devine un token distinct la splitting.
            Rtf::finalizeCurrentSpan(state);
        }
    }
    else if (word == L"footer") {
        // 1. Finalizează orice span/paragraf precedent
        finalizeCurrentSpan(state);

        // 2. Setează flag-ul pentru parsarea footer-ului
        state.isParsingFooter = true;
        state.isParsingHeader = false;

        // 3. Resetarea stării de paragraf curent
        // Un nou paragraf va fi creat la următorul text sau \pard.

        LOG_SUCCESS(L"INTRU IN FOOTER");
    }
    // Similar pentru \header...
    else if (word == L"header") {
        finalizeCurrentSpan(state);
        state.isParsingHeader = true;
        state.isParsingFooter = false;
        LOG_SUCCESS(L"INTRU IN HEADER");
    }
    else if (word == L"qc") {
        finalizeCurrentSpan(state); // Finalizează Span-ul înainte de a aplica alinierea
        if (!state.currentParagraph) {
            LOG_ERROR(L"CRASH PREVENIT: Paragraful curent este NULL după finalizeCurrentSpan. Re-creare.");
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }
        state.currentStyle.textAlign = L"center";
        if (state.currentParagraph) {
            state.currentParagraph->style = state.currentStyle; // Aplică stilul la Paragraf
        }
    }
    else if (word == L"qr") {
        finalizeCurrentSpan(state); // Finalizează Span-ul înainte de a aplica alinierea
        if (!state.currentParagraph) {
            LOG_ERROR(L"CRASH PREVENIT: Paragraful curent este NULL după finalizeCurrentSpan. Re-creare.");
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }
        state.currentStyle.textAlign = L"right";
        if (state.currentParagraph) {
            state.currentParagraph->style = state.currentStyle; // Aplică stilul la Paragraf
        }
    }
    else if (word == L"ql") {
        finalizeCurrentSpan(state); // Finalizează Span-ul înainte de a aplica alinierea
        if (!state.currentParagraph) {
            LOG_ERROR(L"CRASH PREVENIT: Paragraful curent este NULL după finalizeCurrentSpan. Re-creare.");
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }
        state.currentStyle.textAlign = L"right";
        if (state.currentParagraph) {
            state.currentParagraph->style = state.currentStyle; // Aplică stilul la Paragraf
        }
    }
    else if (word == L"qj") {
        finalizeCurrentSpan(state); // Finalizează Span-ul înainte de a aplica alinierea
        if (!state.currentParagraph) {
            LOG_ERROR(L"CRASH PREVENIT: Paragraful curent este NULL după finalizeCurrentSpan. Re-creare.");
            state.currentParagraph = std::make_unique<RtfParagraph>();
        }
        state.currentStyle.textAlign = L"justify";
        if (state.currentParagraph) {
            state.currentParagraph->style = state.currentStyle; // Aplică stilul la Paragraf
        }
    }
    
    // --- 3. COMANDĂ DE TABEL ---
    else if (word == L"trowd") {
        // A. Finalizează paragraful precedent care nu era în tabel.
        auto block = finalizeCurrentParagraph(state);
        if (block) {
            parsedBlocks.push_back(std::move(block));
        }

        // B. Inițializează starea rândului
        if (!state.currentTable) {
            state.currentTable = std::make_unique<RtfTable>();
        }

        state.inTable = true;
        state.currentRow = RtfRow{}; // Resetează rândul de lucru

        // C. NU creezi celulă aici. Bordurile pentru prima celulă (Celulă 0) urmează.
        // Celula 0 va fi creată de \cellx, \cell sau \intbl (dacă este implementat).
        // În acest caz, celula 0 va fi creată la următoarea comandă \cellx (care este și definiția celulei).
        state.currentCellIndex = 0; // <-- NOU: Setează indexul la 0
        state.currentCell = nullptr; // Resetează-l, următorul \cellx/definiție îl va seta.
    }
    // --- 4. COMANDĂ DE TABEL (Continuare/Start Conținut) ---
    else if (word == L"intbl") {
        if (!state.inTable) return;

        // ⭐ COREȘCȚIE: Setăm pointerul la celula CURENTĂ (indexul ar trebui să fie 0 după \trowd)
        if (state.currentCell == nullptr && state.currentCellIndex >= 0 &&
            state.currentCellIndex < state.currentRow.cells.size())
        {
            // Ne atașăm la celula deja creată de \cellx la indexul corect.
            state.currentCell = &state.currentRow.cells[state.currentCellIndex];
        }
        // ... (restul logicii existente - finalizare paragraf și creare unul nou) ...
        finalizeCurrentParagraph(state);
        state.currentParagraph = std::make_unique<RtfParagraph>();
        state.currentParagraph->style = state.currentStyle;
    }
    else if (word.rfind(L"cellx", 0) == 0) {
    if (!state.inTable || !state.currentTable) return;

    // ⭐ UTILIZĂM PARAMETRUL NUMERIC TRIMIS DE PARSER (N-ul din \cellxN)
    int twips = param;

    // Verificare rapidă: \cellx trebuie să aibă un număr pozitiv ca parametru
    if (twips <= 0) {
        LOG_WARNING(L"WARNING CELLX: Parametru invalid (<= 0) pentru cellx. Ignorat.");
        return;
    }

    // 1. Asigură-te că celula pe care o finalizăm este deja creată.
    if (state.currentCell == nullptr) {
        state.currentRow.cells.emplace_back();
        state.currentCell = &state.currentRow.cells.back();
        LOG_DEBUG(L"DEBUG CELLX: Celula noua creata la " + std::to_wstring(state.currentRow.cells.size()));
    }

    // 2. Aplică bordurile acumulate la celula curentă.
    state.currentCell->borders = state.currentCellBorders;
    state.currentCellBorders = CellBorders(); // Resetăm specificațiile de bordură.

    state.currentCell->padding = state.currentCellPadding;
    state.currentCellPadding = {};

    LOG_DEBUG(L"DEBUG CELLX: Borduri aplicate la celula curenta.");

    // 3. Adaugă lățimea la lista de lățimi de coloane (doar pentru primul rând)
    if (state.currentTable->rows.empty()) {
        float widthPt = static_cast<float>(twips) / 20.0f;
        state.currentTable->columnWidthsPt.push_back(widthPt);
        LOG_DEBUG(L"DEBUG CELLX: Latime coloana adaugata: " + std::to_wstring(widthPt) + L"pt.");
    }

    // 4. Finalizează Celula Curentă.
    state.currentCell = nullptr;
    LOG_DEBUG(L"DEBUG CELLX: Celula finalizata (pointer setat la NULL).");
}
    else if (word == L"cell") {
        if (!state.inTable) return;

        // A. Finalizează conținutul celulei precedente
        finalizeCurrentParagraph(state);

        // B. Finalizează celula precedentă.
        state.currentCell = nullptr;

        // ⭐ COREȘCȚIE CRITICĂ: Avansăm la următoarea celulă.
        state.currentCellIndex++;

        // C. Resetăm specificațiile de bordură acumulate
        state.currentCellBorders = CellBorders();

        // D. Nu creăm un paragraf nou, ci ne așteptăm ca următorul \intbl să inițieze conținutul 
        // celei de-a N+1-a celule, utilizând noul index.
    }
    else if (word == L"row") {
        if (!state.inTable || !state.currentTable) return;

        
            // Dacă suntem în footer, mutăm întregul tabel
            if (state.isParsingFooter) {
                this->footerBlocks.push_back(std::move(state.currentTable));
                state.currentTable = nullptr;
                state.inTable = false;
                // Nu uitați să resetați și starea de paragraf curent
                state.currentParagraph = std::make_unique<RtfParagraph>();
                return;
            }
        


        // A. Finalizează conținutul din ultima celulă a rândului
        finalizeCurrentParagraph(state);

        // -----------------------------------------------------------------
        // ⭐ CORECȚIE FINALĂ: Verifică și elimină celula goală reziduală
        // Aceasta rezolvă problema Celulelor 4
        // -----------------------------------------------------------------
        if (state.currentCell && state.currentCell->content.empty() && !state.currentRow.cells.empty()) {
            // Dacă ultima celulă e goală (creată de ultimul \cell înainte de \row), o eliminăm.
            state.currentRow.cells.pop_back();
        }

        // B. Mută rândul completat în tabelul curent
        state.currentTable->rows.push_back(std::move(state.currentRow));

        // C. Resetarea stării
        state.currentRow = RtfRow{};
        state.currentCell = nullptr;
        state.currentCellIndex = -1; // <-- NOU: Resetăm indexul la -1.

        // D. Recreează un paragraf NOU pentru textul care ar putea urma DUPĂ tabel
        state.currentParagraph = std::make_unique<RtfParagraph>();
    }
    // Dimensiunile paginii sunt în TWIPS (1/20 dintr-un punct)

    else if (word == L"ansicpg") {
        state.ansicpg = param; // Setează code page curent (1250)
        LOG_DEBUG(L"Setează code page: " + std::to_wstring(param));
    }
    // \paperw<N>: Lățimea paginii în twips
    else if (word == L"paperw") {
        LOG_DEBUG(L"Setează lățimea paginii: " + std::to_wstring(param) + L" twips");
        state.pageConfig.setWidthTwips(param);
    }
    // \paperh<N>: Înălțimea paginii în twips
    else if (word == L"paperh") {
        LOG_DEBUG(L"Setează înălțimea paginii: " + std::to_wstring(param) + L" twips");
        state.pageConfig.setHeightTwips(param);
    }
    // \lndscpsxn: Setează orientarea la Landscape
    else if (word == L"lndscpsxn") {
        LOG_DEBUG(L"Setează orientarea: Landscape");
        state.pageConfig.setOrientation(RtfOrientation::Landscape);
    }
    // \margl<N>: Marginea stângă în twips
    else if (word == L"margl") {
        LOG_DEBUG(L"Setează marginea stângă: " + std::to_wstring(param) + L" twips");
        // Presupunem că RtfPage::setMargins va fi actualizată sau folosim setteri individuali
        // Pentru simplitate, actualizăm toți setterii RtfPage

        // Dacă RtfPage::setMargins nu este disponibil, folosiți direct setter-ul:
        // state.pageConfig.setMargins(state.pageConfig.getMarginTopTwips(), ..., ..., param); 
        // Sau, mai simplu:
        state.pageConfig.setMargins(
            state.pageConfig.getMarginTopTwips(),
            state.pageConfig.getMarginRightTwips(),
            state.pageConfig.getMarginBottomTwips(),
            param // Marginea Stângă (nouă)
        );
    }
    // \margr<N>: Marginea dreaptă în twips
    else if (word == L"margr") {
        LOG_DEBUG(L"Setează marginea dreaptă: " + std::to_wstring(param) + L" twips");
        state.pageConfig.setMargins(
            state.pageConfig.getMarginTopTwips(),
            param, // Marginea Dreaptă (nouă)
            state.pageConfig.getMarginBottomTwips(),
            state.pageConfig.getMarginLeftTwips()
        );
    }
    
    // \margt<N>: Marginea de sus în twips
    else if (word == L"margt") {
        LOG_DEBUG(L"Setează marginea de sus: " + std::to_wstring(param) + L" twips");
        state.pageConfig.setMargins(
            param, // Marginea de Sus (nouă)
            state.pageConfig.getMarginRightTwips(),
            state.pageConfig.getMarginBottomTwips(),
            state.pageConfig.getMarginLeftTwips()
        );
    }
    // \margb<N>: Marginea de jos în twips
    else if (word == L"margb") {
        LOG_DEBUG(L"Setează marginea de jos: " + std::to_wstring(param) + L" twips");
        state.pageConfig.setMargins(
            state.pageConfig.getMarginTopTwips(),
            state.pageConfig.getMarginRightTwips(),
            param, // Marginea de Jos (nouă)
            state.pageConfig.getMarginLeftTwips()
        );
    }
    else if (word == L"f" && param != -1) { // Tratează \fN (f0, f1, etc.)
        long fontIndex = param;

        // 1. Cazul de parsare: Suntem în secțiunea de metadate \fonttbl
        if (state.parsingFontTable) {

            // Stochează indicele fontului pentru ca handler-ul de text să poată atribui numele
            state.currentFontIndexForTable = fontIndex;

            LOG_DEBUG(L"FONT TABLE: Index font curent stocat: " + std::to_wstring(fontIndex));

        }
    
        // 2. Cazul de aplicare a stilului: Suntem în corpul documentului
        else {

            // Caută numele fontului în tabela populată anterior
            if (state.fontTable.count(fontIndex)) {

                std::wstring fontName = state.fontTable.at(fontIndex);

                // Aplicarea stilului:
                finalizeCurrentSpan(state);
                state.currentStyle.fontFamily = fontName;

                if (state.currentParagraph) {
                    state.currentParagraph->style.fontFamily = fontName;
                }

                LOG_DEBUG(L"DEBUG FONT: Font setat (Aplicare) la: " + fontName + L" (index: " + std::to_wstring(fontIndex) + L")");
            }
            else {
                LOG_WARNING(L"WARNING FONT: Index font " + std::to_wstring(fontIndex) + L" nu este definit. Nu se aplică fontul.");
            }
        }
    }
    else if (word == L"chpgn") {
        // Închide span-ul curent (dacă există text în el)
        finalizeCurrentSpan(state);

        // ⭐ Adaugă marcatorul \chpgn ca text într-un span separat.
        // Acesta va crea un nou span dedicat doar pentru acest marcator.
        // Această tehnică este comună pentru câmpurile care nu au conținut afișat imediat.
        state.currentTextBuffer += L"\\chpgn";

        finalizeCurrentSpan(state); // Finalizează span-ul cu marcatorul

        return; // Ieși după procesare
    }
   
    // ... (restul comenzilor RTF) ...
   // --- 5. COMANDĂ DOCUMENT/IGNORARE (Curățarea log-ului) ---
    else if (word == L"trgaph" || word == L"rtf" || word == L"ansi" || word == L"ansicpg" || word == L"deff" || word == L"cf" || word == L"sa" || word == L"fonttbl") {
    // Aceste comenzi sunt fie globale (rtf, ansi), metadate (fonttbl, cf), fie setări pe care le ignorăm (ansicpg, deff, sa).
    // Le logăm ca ignorate pentru a nu le eticheta ca "necunoscute".
    LOG_DEBUG(L"Rtf::handleControlWord: Comandă document/ignorată: " + word);
    }
    else {
        LOG_DEBUG(L"Rtf::handleControlWord: cuvânt de control necunoscut: " + word);
    }
}

void Rtf::finalizeCurrentSpan(RtfParseState& state) {
    // Creează un Span doar dacă există un paragraf în lucru și text în buffer
    if (state.currentParagraph && !state.currentTextBuffer.empty()) {
        RtfSpan newSpan;
        newSpan.text = state.currentTextBuffer;

        // Span-ul moștenește stilul curent în momentul finalizării
        newSpan.style = state.currentStyle;

        // Mută Span-ul în paragraful curent
        state.currentParagraph->spans.push_back(std::move(newSpan));

        // Golește buffer-ul de text pentru următorul Span
        state.currentTextBuffer.clear();
    }
}

void Rtf::applyPendingBorders(RtfParseState& state) {
    // Aplică specificația curentă (\brdrs, \brdrw) la pozițiile marcate anterior
    if (state.currentBorderSpec.isSet()) {
        if (state.borderLeftPending) {
            state.currentCellBorders.left = state.currentBorderSpec;
            state.borderLeftPending = false;
        }
        if (state.borderTopPending) {
            state.currentCellBorders.top = state.currentBorderSpec;
            state.borderTopPending = false;
        }
        if (state.borderBottomPending) {
            state.currentCellBorders.bottom = state.currentBorderSpec;
            state.borderBottomPending = false;
        }
        if (state.borderRightPending) {
            state.currentCellBorders.right = state.currentBorderSpec;
            state.borderRightPending = false;
        }
    }
}
#pragma once
#include "xhtml.hpp"
#include "../wrapper/PdfWriterWrapper.hpp" 

#include <stack>


struct ElementContext {
    XhtmlElement element;
    Style style;
    BoxMetrics metric;
};

class PdfConverter {
private:
    const Xhtml& m_xhtml;
    PdfWriterWrapper m_pdfWriter;

    // ⭐ MIEZUL: Contextul de randare CURENT.
    RenderingContext m_context;

    // Stiva pentru a salva/restaura starea contextului la intrarea/ieșirea din elementele block.
    std::stack<RenderingContext> m_contextStack;
   
    std::vector<std::unique_ptr<Table>> m_tableStorage;

    // Obiectul Page este gestionat separat (poate fi membru sau poate fi gestionat de un PageManager)
    Page m_currentPage;

    std::stack<ElementContext> m_activeBlockStack;
    // ... Alte proprietăți necesare (ex: Font Manager, Mapări CSS)

    std::vector<LineToken> m_lineBuffer;


    std::list<Page> m_documentPages;

public:
    // Constructorul setează contextul inițial pe baza paginii
    PdfConverter(const Xhtml& xhtml);
    bool convert(const std::wstring& outputFilePath);
    void clean();
    //void printRenderingContext(int stackLevel = 0) const;
private:

    void newLineAndCheckPageBreak();

    // ====================================================================
    // 🎯 GRUP 1: DISPATCHER ȘI GESTIUNEA FLOW-ULUI (Moștenire și Recursivitate)
    // ====================================================================

    // Metoda recursivă principală (Dispatcher)
    void processNodeRecursive(const XhtmlElement& element);

    // Salvează contextul, aplică moștenirea și resetază cursorii pentru Block.
    void pushBlockContext(const XhtmlElement& element);

    // Restaurează contextul anterior.
    void popBlockContext(double next_element_y_from_child);

    // Citește CSS-ul elementului (inline + selectori) și îl aplică pe m_context.style.
    void applyCssToContext(const XhtmlElement& element);
    void applyCssToStyle(const XhtmlElement& element, Style& target_style);

    // ====================================================================
    // 🎯 GRUP 2: RANDARE ELEMENTE ȘI PRIMITIVE
    // ====================================================================

    // Logica de wrapping și desenare text. Folosește m_context.style.
    //void drawText(const std::wstring& text);

    // Logica de randare specifică tag-urilor
    //void processBlockFrame(const XhtmlElement& element);
    void processTextNode(const XhtmlElement& element);
    void processBody(const XhtmlElement& element);
    void processHtml(const XhtmlElement& element);
    void processDiv(const XhtmlElement& element);
    void processP(const XhtmlElement& element);

    void processImg(const XhtmlElement& element);
    void processImage(const XhtmlElement& element);

    void processSpan(const XhtmlElement& element); // Element inline
    void processB(const XhtmlElement& element);
    void processI(const XhtmlElement& element);
    void processH1(const XhtmlElement& element);
    void processBR(const XhtmlElement& element);

    void processTable(const XhtmlElement& element);
    void processTableRow(const XhtmlElement& element);
    void processTableCell(const XhtmlElement& element);
    void buildTableStructureRecursive(const XhtmlElement& parent, Table& table_data);
    void calculateTableLayout(Table& table_data);
    

    void processGenericBlock(const XhtmlElement& element); // Div-uri simple
    void processGenericInline(const XhtmlElement& element); // Div-uri simple
  
    // Procesarea nodurilor text
    void processContentAsText(const XhtmlElement& textElement, double max_content_x_limit = -1.0);
    void processTextContent(const std::wstring& textContent);
    // Flow control
    void processLineBreak();
    bool isBlockElement(const std::wstring& tagName) const;


    void finalizeAndPaint();
    void paintCurentPage();
    void paintPage(Page &page);

    void newPage();
    void checkPageBreak(double required_height);
    RenderInstruction createSpanningBoxInstruction(
        double x_start,
        double y_start,
        double width,
        const Style& style);
    // ====================================================================
    // 🎯 GRUP 3: LOGICĂ TABELE (Two-Pass Layout)
    // ====================================================================
/*
    void processTable(const XhtmlElement& element);

    // PASUL 1: Măsurare (populează TableRenderData)
    bool collectTableDataAndSetup(const XhtmlElement& element, TableRenderData& tableData);
    void calculateCellDimensions(TableRenderData& tableData);

    // PASUL 2: Randare
    void drawTableStructure(const TableRenderData& tableData);
    void renderCellContents(const TableRenderData& tableData);
*/
// ====================================================================
// 🎯 GRUP 4: UTILITY ȘI CONVERSIE
// ====================================================================
   
    BoxMetrics calculateBoxMetrics(double availableWidth,
        double x_flow_start,
        double y_flow_start,
        double content_height_calculated ); // Folosit pentru 'height: auto'
// ... (alte utilitare)
};



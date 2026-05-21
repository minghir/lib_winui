// RtfToPdfConverter.hpp (Probabil în tdocs/ sau converters/)
#pragma once

#include "../tdocs/rtf.hpp" // Pentru Rtf, RtfBlock, RtfParagraph, etc.
#include "../tdocs/BasePdfConverter.hpp" // Clasa de bază
#include "../wrapper/PdfWriterWrapper.hpp" // Clasa pentru scrierea în PDF
#include <memory>
#include <vector>
#include <functional>

#define TAB_WIDTH 18.

class RtfToPdfConverter : public BasePdfConverter {
private:

    double m_pageHeight;
    double m_contentWidth;
    double m_currentY;
    double m_currentX;
    int m_currentPageNumber = 0;
    int m_totalPagesCount = 0;
    double m_marginLeft;
    double m_marginRight;

    double pageWidth = 0.;
    double pageHeight = 0.;
    std::wstring outputFilePath = L"";

    std::vector<RenderInstruction> m_renderQueue;

    // ⭐ Mapează Numele variabilei RTF la valoarea sa finală (string)
    std::map<std::wstring, std::function<std::wstring(int)>> m_globalVarResolvers;
    

    PdfWriterWrapper m_pdfWriter;
    const Rtf& m_rtfDocument; // Referință la modelul de document parsat

    // Funcții de randare structurală (Flow)
    void processRtfBlock(const RtfBlock& block);
    double  processRtfParagraph(const RtfParagraph& paragraph);
    void processRtfTable(const RtfTable& table);
    void processRtfRow(const RtfRow& row, const std::vector<double>& colWidths);
    double processRtfCell(const RtfCell& cell, double cellStartX, double cellStartY, double cellWidth, double cellHeight);
    void processRtfSpan(const RtfSpan& span);

    // Funcții ajutătoare (Auxiliare)
    void applyStyleToWriter(const Style& style);
    void newLineAndCheckPageBreak(double requiredHeight);


    double calculateLineXStart(const std::wstring& alignment, double contentWidth, double lineWidth, double marginLeft) const;
    void renderSpans(const std::vector<const RtfSpan*>& spans, double startX, double yBaseline);
    void renderWords(const std::vector<std::pair<std::wstring, Style>>& words, const std::wstring& alignment, double lineContentWidth, double lineHeight);
    void renderCellBorder(const BorderSpec& spec, double x1, double y1, double x2, double y2);

    void renderFooter();
    double renderFooterParagraph(const RtfParagraph& paragraph, double lineHeight);
    double renderFooterTable(const RtfTable& table);
    double calculateXOffsetForAlignment(const std::wstring& align, double cellWidth, double contentWidth);
    std::wstring replaceRtfFields(const std::wstring& text, int currentPage, int totalPages);

    bool finalizeAndPaint();

    void initializeGlobalVarResolvers();
    void identifyGlobalVars(const std::wstring& text, std::vector<std::wstring>& vars);
    std::wstring resolveTextContent(const std::wstring& content, const std::vector<std::wstring>& vars);
public:
    // Constructorul primește obiectul Rtf gata parsat
    RtfToPdfConverter(const Rtf& rtfDocument)
        : m_rtfDocument(rtfDocument) {
        // Inițializări specifice RTF, dacă e cazul
    }

    // Metoda principală care lansează procesul de randare
    bool convert(const std::wstring& filename);


};
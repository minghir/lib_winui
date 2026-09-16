#ifndef WIN_DIAG_H
#define WIN_DIAG_H

#include "win_main.h"
#include "vPdfViewerWindow.hpp"
#include "vPdfViewer.hpp"
#include <string>
#include <vector>

static vPdfViewer* g_pPdfViewer = nullptr;

void OpenPdfDiag(HWND parent, std::wstring pdf_file_path);

void OpenPdfDiagFromMemory(HWND parent, const std::vector<uint8_t>& pdf_buffer);

void ShutdownPdfDiag(HWND parent);


void OpenPdfWinFromMemory(HINSTANCE hInstance, HWND parent, EventDispatcher& dispatcher, const std::vector<uint8_t>& pdf_buffer);
void OpenPdfWinFromFile(HINSTANCE hInstance, HWND parent, EventDispatcher& dispatcher, const std::wstring& filePath);

bool IsPdfViewerOpen();
// Închide și eliberează viewer-ul vechi
void ClosePdfViewer();

#endif
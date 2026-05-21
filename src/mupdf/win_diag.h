#ifndef WIN_DIAG_H
#define WIN_DIAG_H

#include "win_main.h"
#include <string>



void OpenPdfDiag(HWND parent, std::wstring pdf_file_path);
void ShutdownPdfDiag(HWND parent);

#endif
#include "win_diag.h"
#include "vPdfViewer.hpp"
#include "../ui/vApp.hpp"

#include <vector>
#include <cstdint>
#include <iostream>
#include <string>
#include <fstream>




void OpenPdfDiag(HWND parent, std::wstring pdf_file_path) {

	//if (dialog_opened) return;

	//dialog_opened = 1;

	//app_close();

	fz_context* ctx = NULL;
	if (!ctx) {
		ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
		pdf_init(ctx);
		if (!ctx) {
			MessageBoxA(NULL, "Cannot initialize MuPDF context.", "MuPDF: Error", MB_OK);
			return ;
		}

		winopen(parent);
		set_filename(pdf_file_path.c_str());
		app_open();
	}
	else {
		MessageBoxA(NULL, "Cannot initialize MuPDF context EXISTA.", "MuPDF: Error", MB_OK);

		app_close();


		ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
		pdf_init(ctx);
		if (!ctx) {
			MessageBoxA(NULL, "Cannot initialize MuPDF context.", "MuPDF: Error", MB_OK);
			return ;
		}
		winopen(parent);
		set_filename(pdf_file_path.c_str());
		app_reopen();
	}

	return ;
}


void ShutdownPdfDiag(HWND parent) {
	//app_close();
}

// Flag global/static pentru a urmări dacă fereastra este deja deschisă
static bool g_is_pdf_open = false;

void OpenPdfDiagFromMemory(HWND parent, const std::vector<uint8_t>& pdf_buffer) {
	// 1. Verificăm dacă raportul este deja deschis
	HWND hFrame = get_frame_win();
	if (g_is_pdf_open && hFrame && IsWindow(hFrame)) {
		// Dacă este minimizată, o restaurăm
		if (IsIconic(hFrame)) {
			ShowWindow(hFrame, SW_RESTORE);
		}

		// Aducem fereastra existentă în prim-plan
		SetForegroundWindow(hFrame);
		BringWindowToTop(hFrame);

		MessageBoxA(hFrame,
			"Un raport PDF este deja deschis!\nÎnchideți raportul curent înainte de a deschide altul.",
			"Avertisment", MB_OK | MB_ICONWARNING);
		return; // OPRIM AICI, NU MAI CREĂM ALTA FEREASTRĂ
	}

	if (pdf_buffer.empty()) {
		MessageBoxA(NULL, "Buffer-ul PDF este gol.", "MuPDF: Error", MB_OK);
		return;
	}

	// 2. Inițializăm ferestrele UI MuPDF DOAR dacă fereastra nu există deja
	if (!hFrame || !IsWindow(hFrame)) {
		winopen(parent);
	}

	// 3. Deschidem documentul din memorie și afișăm fereastra
	int success = app_open_from_memory(
		reinterpret_cast<const unsigned char*>(pdf_buffer.data()),
		pdf_buffer.size()
	);

	if (success) {
		g_is_pdf_open = true; // Setați flag-ul pe true la deschiderea cu succes
	}
	else {
		MessageBoxA(NULL, "Nu s-a putut deschide documentul PDF din memorie.", "MuPDF: Error", MB_OK);
	}
}

/*
void OpenPdfWinFromMemory(HINSTANCE hInstance, HWND parent, EventDispatcher& dispatcher, const std::vector<uint8_t>& pdf_buffer) {
	static vPdfViewerWindow* pPdfWin = nullptr;


	// 1. Dacă fereastra există deja pe ecran, o aducem în față
	if (pPdfWin && pPdfWin->isVisible()) {
		pPdfWin->show();
		MessageBoxA(pPdfWin->getHandle(), "Un raport este deja deschis!", "Avertisment", MB_OK | MB_ICONWARNING);
		return;
	}

	// 2. Distrugem instanța veche dacă era închisă
	if (pPdfWin) {
		delete pPdfWin;
		pPdfWin = nullptr;
	}

	// 3. Creăm instanța vWindow
	pPdfWin = new vPdfViewerWindow(hInstance, "pdf_viewer_win", dispatcher);
	pPdfWin->create(L"VPdfViewerClass", L"Raport ANC PDF", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, parent);

	// 4. Încărcăm PDF-ul direct din memorie
	if (!pPdfWin->loadPdfFromMemory(pdf_buffer)) {
		MessageBoxA(parent, "Nu s-a putut încărca documentul PDF.", "Eroare", MB_OK | MB_ICONERROR);
	}
}
*/



void OpenPdfWinFromMemory(HINSTANCE hInstance, HWND parent, EventDispatcher& dispatcher, const std::vector<uint8_t>& pdf_buffer) {
	static vPdfViewer* pPdfViewer = nullptr;

	// 1. Verificăm dacă fereastra există ȘI handle-ul Win32 este încă valid
	if (pPdfViewer) {
		auto* pWin = pPdfViewer->getWindow();
		if (pWin && pWin->getHandle() && IsWindow(pWin->getHandle()) && pWin->isVisible()) {
			pWin->show();
			SetForegroundWindow(pWin->getHandle());
			MessageBoxA(pWin->getHandle(), "Un raport este deja deschis!", "Avertisment", MB_OK | MB_ICONWARNING);
			return;
		}

		// Dacă fereastra a fost închisă de utilizator, ștergem obiectul vechi în siguranță
		delete pPdfViewer;
		pPdfViewer = nullptr;
	}

	// 2. Creăm instanța nouă
	pPdfViewer = new vPdfViewer(hInstance, dispatcher);

	if (!pPdfViewer->create(parent, L"Raport ANC PDF")) {
		MessageBoxA(parent, "Nu s-a putut crea fereastra PDF Viewer.", "Eroare", MB_OK | MB_ICONERROR);
		delete pPdfViewer;
		pPdfViewer = nullptr;
		return;
	}

	if (!pPdfViewer->loadFromMemory(pdf_buffer)) {
		MessageBoxA(parent, "Nu s-a putut încărca documentul PDF din memorie.", "Eroare", MB_OK | MB_ICONERROR);
	}
}
/*
void OpenPdfWinFromFile(HINSTANCE hInstance, HWND parent, EventDispatcher& dispatcher, const std::wstring& filePath) {
	static vPdfViewerWindow* pPdfWin = nullptr;

	// 1. Dacă fereastra există deja pe ecran, o aducem în față
	if (pPdfWin && pPdfWin->isVisible()) {
		pPdfWin->show();
		MessageBoxA(pPdfWin->getHandle(), "Un raport este deja deschis!", "Avertisment", MB_OK | MB_ICONWARNING);
		return;
	}

	// 2. Distrugem instanța veche dacă era închisă
	if (pPdfWin) {
		delete pPdfWin;
		pPdfWin = nullptr;
	}

	// 3. Creăm instanța vWindow
	pPdfWin = new vPdfViewerWindow(hInstance, "pdf_viewer_win", dispatcher);
	pPdfWin->create(L"VPdfViewerClass", L"Raport ANC PDF", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, parent);

	// 4. Încărcăm PDF-ul direct din fișier
	if (!pPdfWin->loadPdfFromFile(filePath)) {
		MessageBoxA(parent, "Nu s-a putut încărca documentul PDF din fișier.", "Eroare", MB_OK | MB_ICONERROR);
	}
}
*/





// Verifică dacă fereastra PDF este deschisă
bool IsPdfViewerOpen() {
	if (g_pPdfViewer && g_pPdfViewer->getWindow() && g_pPdfViewer->getWindow()->isVisible()) {
		return true;
	}
	return false;
}

// Închide și eliberează viewer-ul vechi
void ClosePdfViewer() {
	if (g_pPdfViewer) {
		delete g_pPdfViewer;
		g_pPdfViewer = nullptr;
	}
}
/*
void OpenPdfWinFromFile(HINSTANCE hInstance, HWND parent, EventDispatcher& dispatcher, const std::wstring& filePath) {
	// 1. Curățăm viewer-ul vechi dacă exista și fereastra a fost închisă
	ClosePdfViewer();

	// 2. Creăm instanța nouă
	g_pPdfViewer = new vPdfViewer(hInstance, dispatcher);

	if (!g_pPdfViewer->create(parent, L"Raport ANC PDF")) {
		MessageBoxA(parent, "Nu s-a putut crea fereastra PDF Viewer.", "Eroare", MB_OK | MB_ICONERROR);
		delete g_pPdfViewer;
		g_pPdfViewer = nullptr;
		return;
	}

	// 3. Citim fișierul în memorie pentru a NU bloca fișierul de pe disc
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			file.close(); // Am închis fișierul imediat! Discul este liber.

			if (g_pPdfViewer->loadFromMemory(buffer)) {
				return;
			}
		}
		file.close();
	}

	// Fallback în caz că citirea în memorie nu a reușit
	if (!g_pPdfViewer->loadFromFile(filePath)) {
		MessageBoxA(parent, "Nu s-a putut încărca documentul PDF.", "Eroare", MB_OK | MB_ICONERROR);
	}
}
*/

void OpenPdfWinFromFile(HINSTANCE hInstance, HWND parent, EventDispatcher& dispatcher, const std::wstring& filePath) {
	// 1. Curățăm viewer-ul vechi dacă exista și fereastra a fost închisă
	ClosePdfViewer();

	// 2. Creăm instanța nouă
	// Dacă parent este NULL, fereastra va fi creată ca fereastră independentă (top-level)
	g_pPdfViewer = new vPdfViewer(hInstance, dispatcher);

	if (!g_pPdfViewer->create(parent, L"Raport ANC PDF")) {
		MessageBoxA(parent, "Nu s-a putut crea fereastra PDF Viewer.", "Eroare", MB_OK | MB_ICONERROR);
		delete g_pPdfViewer;
		g_pPdfViewer = nullptr;
		return;
	}

	// 3. Citim fișierul în memorie pentru a nu bloca discul
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			file.close();

			if (g_pPdfViewer->loadFromMemory(buffer)) {
				return;
			}
		}
		file.close();
	}

	// Fallback în caz că citirea în memorie nu a reușit
	if (!g_pPdfViewer->loadFromFile(filePath)) {
		MessageBoxA(parent, "Nu s-a putut încărca documentul PDF.", "Eroare", MB_OK | MB_ICONERROR);
	}
}
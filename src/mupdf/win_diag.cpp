#include "win_diag.h"



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
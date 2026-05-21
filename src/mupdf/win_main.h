
#ifndef WIN_MAIN_H
#define WIN_MAIN_H



#ifdef __cplusplus
extern "C" {
#endif


#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN

// Include pdfapp.h *AFTER* the UNICODE defines 
#include "pdfapp.h"

#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define ID_BTN_PRINT 1001
#define ID_BTN_EMAIL 1002
#define ID_BTN_SAVE 1003
#define ID_BTN_EXIT 1004

//extern int dialog_opened;

typedef BOOL(SetProcessDPIAwareFn)(void);

void pdf_init(fz_context* ctx );
void install_app(char *argv0);
void winwarn(pdfapp_t *app, char *msg);
void winerror(pdfapp_t *app, char *msg);
void winalert(pdfapp_t *app, pdf_alert_event *alert);
void winprint(pdfapp_t *app);
int winsavequery(pdfapp_t *app);
int winquery(pdfapp_t *app, const char *query);
int winfilename(wchar_t *buf, int len);
int wingetcertpath(pdfapp_t *app, char *buf, int len);
int wingetsavepath(pdfapp_t *app, char *buf, int len);
void winreplacefile(pdfapp_t *app, char *source, char *target);
void wincopyfile(pdfapp_t *app, char *source, char *target);
void winremovefile(char *source);

INT_PTR CALLBACK dlogpassproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK dlogtextproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK dlogchoiceproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

char *winpassword(pdfapp_t *app, char *filename);
char *wintextinput(pdfapp_t *app, char *inittext, int retry);
int winchoiceinput(pdfapp_t *app, int nopts, const char *opts[], int *nvals, const char *vals[]);

INT_PTR CALLBACK dloginfoproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void info();
INT_PTR CALLBACK dlogaboutproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void winhelp(pdfapp_t*app);
void winopen(HWND win_parent);
void do_close(pdfapp_t *app);
void winclose(pdfapp_t *app);
void wincursor(pdfapp_t *app, int curs);
int winisresolutionacceptable(pdfapp_t *app, fz_matrix ctm);
void wintitle(pdfapp_t *app, char *title);
void windrawrect(pdfapp_t *app, int x0, int y0, int x1, int y1);
void windrawstring(pdfapp_t *app, int x, int y, char *s);
void winblitsearch();
void winblit();
void winresize(pdfapp_t *app, int w, int h);
void winrepaint(pdfapp_t *app);
void winrepaintsearch(pdfapp_t *app);
void winfullscreen(pdfapp_t *app, int state);
void windocopy(pdfapp_t *app);

void winreloadpage(pdfapp_t *app);
void winopenuri(pdfapp_t *app, char *buf);
void winadvancetimer(pdfapp_t *app, float delay);
void killtimer(pdfapp_t *app);
void handlekey(int c);
void handlemouse(int x, int y, int btn, int state);
LRESULT CALLBACK frameproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK viewproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int get_system_dpi(void);
void usage(const char *argv0);
HWND get_frame_win();
int set_filename(const wchar_t* inputFilename);
void app_open();
void app_reopen();
void app_close();

//int winemail();

#ifdef __cplusplus
}
#endif



#endif
